#include "muon/integration.h"

#include <limits>

#include "glog/logging.h"
#include "muon/hemisphere_sampling.h"
#include "muon/lighting.h"
#include "muon/transform.h"
#include "third_party/glm/gtc/constants.hpp"
#include "third_party/glm/gtx/component_wise.hpp"

namespace muon {

glm::vec3 Integrator::Trace(const Ray &ray) {
  return Trace(ray, glm::vec3(1.0f), 0);
}

glm::vec3 Integrator::Trace(const Ray &ray, const glm::vec3 &throughput,
                            const int depth) {
  // When Russian Roulette is enabled, we rely on it to probabilistically end
  // paths. Currently, max_depth must be -1 when Russian Roulette is enabled in
  // order to result in an unbiased render.
  // When Next Event Estimation is active, we shorten paths by 1, since NEE
  // effectively increases path lengths by one since it samples direct
  // lighting.
  if (scene_.max_depth != -1 &&
      depth > (scene_.next_event_estimation ? scene_.max_depth - 1
                                            : scene_.max_depth)) {
    return glm::vec3(0.0f);
  }
  if (depth == 0) {
    workspace_->stats.IncrementPrimaryRays();
  } else {
    workspace_->stats.IncrementSecondaryRays();
  }
  absl::optional<Intersection> hit =
      scene_.root->Intersect(workspace_.get(), ray);
  if (hit) {
    return Shade(hit.value(), ray, throughput, depth);
  }
  return glm::vec3(0.0f);
}

glm::vec3 NormalsTracer::Shade(const Intersection &hit, const Ray &ray,
                               const glm::vec3 &throughput, const int depth) {
  // Map from [-1, 1] to [0, 1].
  return hit.normal * 0.5f + 0.5f;
}

std::unique_ptr<Integrator> NormalsTracer::Clone() const {
  return absl::make_unique<NormalsTracer>(*this);
}

glm::vec3 DepthTracer::Shade(const Intersection &hit, const Ray &ray,
                             const glm::vec3 &throughput, const int depth) {
  // Map distance from [0, inf] to [1, 0].
  // TODO: This often leads to dark images; is there a way to normalize?
  // Add a configurable scale factor to hit.distance?
  return glm::vec3(1.0f / (1.0f + hit.distance));
}

std::unique_ptr<Integrator> DepthTracer::Clone() const {
  return absl::make_unique<DepthTracer>(*this);
}

glm::vec3 Raytracer::Shade(const Intersection &hit, const Ray &ray,
                           const glm::vec3 &throughput, const int depth) {
  glm::vec3 color = hit.obj->material->ambient + hit.obj->material->emission;

  // Shift the collision point by an epsilon to avoid surfaces shadowing
  // themselves.
  glm::vec3 shift_pos = hit.pos + kEpsilon * hit.normal;

  // Trace the light contributions.
  for (const auto &light : scene_.lights()) {
    ShadingInfo info = light->ShadingInfoAt(hit.pos);
    Ray shadow_ray(shift_pos, info.direction);
    if (scene_.root->HasIntersection(workspace_.get(), shadow_ray,
                                     info.distance)) {
      // Light is occluded.
      continue;
    }
    // Apply a simple Blinn Phong shading model.
    glm::vec3 diffuse_cmp =
        hit.obj->material->diffuse *
        glm::max(glm::dot(hit.normal, info.direction), 0.0f);

    glm::vec3 half_vector = glm::normalize(info.direction - ray.direction());
    glm::vec3 specular_cmp =
        hit.obj->material->specular *
        glm::pow(glm::max(glm::dot(hit.normal, half_vector), 0.0f),
                 hit.obj->material->shininess);

    color += info.color * (diffuse_cmp + specular_cmp);
  }

  // Trace reflectance if the object has any specularity.
  if (glm::any(
          glm::greaterThan(hit.obj->material->specular, glm::vec3(0.0f)))) {
    Ray reflected_ray(shift_pos, Reflect(ray.direction(), hit.normal));

    glm::vec3 reflected_color = Trace(reflected_ray, throughput, depth + 1);
    color += hit.obj->material->specular * reflected_color;
  }

  return color;
}

std::unique_ptr<Integrator> Raytracer::Clone() const {
  return absl::make_unique<Raytracer>(*this);
}

glm::vec3 AnalyticDirect::Shade(const Intersection &hit, const Ray &ray,
                                const glm::vec3 &throughput, const int depth) {
  // For physically based rendering, the rendering equation defines the
  // reflected radiance:
  //   L_r(w_o) = L_e(w_o) + ∫_omega(f(w_i, w_o) * L_i(w_i) * (n • w_i) * dw_i)
  // where L_r is the reflected radiance, L_e is the emitted radiance, L_i is
  // the incident radiance along some incident direction w_i, w_o is the
  // outgoing direction, f() is the BRDF, n is the surface normal, and ∫_omega
  // is the integral over all directions in the unit hemisphere.
  //
  // For this integrator, we care only about direct lighting, so we can  treat
  // the incident radiance L_i as being independent of any direction. We also
  // assume that the shading surface has a Lambertian BRDF which is constant
  // for any w_i and w_o, yielding:
  //   L_d(w_o) = f * ∫_omega(L_i * (n • w_i) * dw_i)
  //
  // The integral above is known as the irradiance, and it turns out there is
  // an analytic form for the irradiance at a point from a polygonal light
  // source, as follows:
  //   L_d(w_o) = f * L_i * (phi(r) • n)
  // where r is the illuminated position and phi(r) is the "irradiance vector".
  //
  // We can compute phi(r) as:
  //   phi(r) = 0.5 * ∑(theta_i(r) * gamma_i(r))
  // where theta_i is the angle subtended by the i-th edge of the polygon, and
  // gamma_i is the unit normal of the polygonal cone. We can compute both of
  // these from the vertices of the polygonal light.

  // Only consider emission for base lighting.
  glm::vec3 color = hit.obj->material->emission;

  // Trace the light contributions.
  for (const auto &light : scene_.lights()) {
    ShadingInfo info = light->ShadingInfoAt(hit.pos);
    // Skip non-area lights.
    if (info.area == nullptr) {
      continue;
    }

    // Vertices are in counter-clockwise order around the quad.
    constexpr int kNumVertices = 4;
    glm::vec3 vertices[kNumVertices];
    vertices[0] = info.area->corner;
    vertices[1] = info.area->corner + info.area->edge0;
    vertices[2] = info.area->corner + info.area->edge0 + info.area->edge1;
    vertices[3] = info.area->corner + info.area->edge1;

    // Divide by pi for energy conservation.
    glm::vec3 diffuse_brdf =
        hit.obj->material->diffuse * glm::one_over_pi<float>();
    glm::vec3 incident_radiance = info.color;

    // Compute phi(r).
    glm::vec3 irradiance_vector(0.0f);

    for (int i = 0; i < kNumVertices; ++i) {
      int next_i = (i + 1) % kNumVertices;
      glm::vec3 u_i = glm::normalize(vertices[i] - hit.pos);
      glm::vec3 u_next_i = glm::normalize(vertices[next_i] - hit.pos);

      // TODO: Does this work if the light isn't facing the surface?
      float theta = glm::acos(glm::dot(u_i, u_next_i));
      glm::vec3 gamma = glm::normalize(glm::cross(u_i, u_next_i));
      irradiance_vector += theta * gamma;
    }
    irradiance_vector *= 0.5f;

    glm::vec3 diffuse_radiance = diffuse_brdf * incident_radiance *
                                 glm::dot(irradiance_vector, hit.normal);
    color += diffuse_radiance;
  }

  return color;
}

std::unique_ptr<Integrator> AnalyticDirect::Clone() const {
  return absl::make_unique<AnalyticDirect>(*this);
}

glm::vec3 MonteCarloDirect::Shade(const Intersection &hit, const Ray &ray,
                                  const glm::vec3 &throughput,
                                  const int depth) {
  // For physically based rendering, the rendering equation defines the
  // reflected radiance:
  //   L_r(w_o) = L_e(w_o) + ∫_omega(f(w_i, w_o) * L_i(w_i) * (n • w_i) * dw_i)
  // where L_r is the reflected radiance, L_e is the emitted radiance, L_i is
  // the incident radiance along some incident direction w_i, w_o is the
  // outgoing direction, f() is the BRDF, n is the surface normal, and ∫_omega
  // is the integral over all directions in the unit hemisphere.
  //
  // For this integrator, we care only about direct lighting, so we only
  // compute the incident light in terms of direct contributions from a light
  // source, while taking into account a visibility term for shadows. For
  // convenience, we can also integrate over the light's area instead of
  // integrating over solid angle:
  //   L_d(w_o) =
  //       L_i * ∫_P(f(w_i, w_o) * (n • w_i) * V(w_i) * (n_l • w_i) / R^2 * dA)
  // where V(w_i) is the visibility term and (n_l • w_i) / R^2 is the
  // normalization term between solid angle and light area.
  //
  // Finally, we can use a Monte Carlo estimator to approximate the integral:
  //   L_d(w_o) =
  //       L_i * A / N * ∑(f(w_i, w_o) * (n • w_i) * V(w_i) * (n_l • w_i) / R^2)
  // where A is the area of the light and N is the number of samples.

  // Only consider emission for base lighting.
  glm::vec3 color = hit.obj->material->emission;

  // Shift the collision point by an epsilon to avoid surfaces shadowing
  // themselves.
  glm::vec3 shift_pos = hit.pos + kEpsilon * hit.normal;

  // Trace the direct light contributions.
  color += ShadeDirect(hit, shift_pos, ray, throughput);

  return color;
}

glm::vec3 MonteCarloDirect::ShadeDirect(const Intersection &hit,
                                        const glm::vec3 &shift_pos,
                                        const Ray &ray,
                                        const glm::vec3 &throughput) {
  glm::vec3 color(0.0f);

  // Calculate direct lighting contributions .
  for (const auto &light : scene_.lights()) {
    ShadingInfo info = light->ShadingInfoAt(hit.pos);
    // Special case for non-area lights.
    // TODO: I'm unconvinced that this is physically accurate...
    if (info.area == nullptr) {
      Ray shadow_ray(shift_pos, info.direction);
      if (scene_.root->HasIntersection(workspace_.get(), shadow_ray,
                                       info.distance)) {
        // Light is occluded.
        continue;
      }
      glm::vec3 irradiance = info.color;
      if (info.distance < std::numeric_limits<float>::infinity()) {
        irradiance /= info.distance * info.distance;
      }
      float cos_incident_angle =
          glm::max(glm::dot(hit.normal, info.direction), 0.0f);
      glm::vec3 brdf_term = hit.obj->material->BRDF().Eval(
          info.direction, ray.direction(), hit.normal);

      color += irradiance * cos_incident_angle * brdf_term;
      continue;
    }

    // Calculate the light's area and reverse normal, which are used in the
    // following calculations. Note, the reverse normal is the direction _away_
    // from which the light is emitting. We do this so that we normalize our
    // sampling method: instead of sampling over the solid angle subtended by
    // the light, we want to sample of the area of the light, and thus need to
    // normalize by:
    //   (n_l • w_i) / R^2
    // where R is the distance between the point being shaded and the
    // corresponding point on the light, w_i is the direction _to_ the point on
    // the light, and thus n_l has to be the reverse normal of the light so
    // that the result of the dot product is positive (alternatively, we could
    // use the reverse of w_i with the normal).
    float light_area =
        glm::length(glm::cross(info.area->edge0, info.area->edge1));
    glm::vec3 light_reverse_normal =
        glm::normalize(glm::cross(info.area->edge0, info.area->edge1));

    // Sum the contributions of all samples. If enabled, we subdivide the
    // light's area via stratified sampling. Note, we assume that the number of
    // requested samples must be a perfect square in this case, and we use this
    // to calculate the dimensions of the strata. When stratified sampling is
    // enabled, each section is sampled once.
    int strata = scene_.light_stratify ? glm::sqrt(scene_.light_samples) : 1;
    int samples_per_section = scene_.light_stratify ? 1 : scene_.light_samples;
    glm::vec3 sample_contributions(0.0f);

    for (int i = 0; i < strata; i++) {
      for (int j = 0; j < strata; j++) {
        for (int k = 0; k < samples_per_section; k++) {
          // Generate a random sample on the surface of the light. When
          // stratified sampling is enabled, this scales the u, v random values
          // by the size of each strata and offsets into the current section
          // that we're sampling from.
          float u = rand_.Next();
          float v = rand_.Next();
          assert(u >= 0 && u < 1 && v >= 0 && v < 1);
          glm::vec3 light_pos = info.area->corner +
                                (i + u) / strata * info.area->edge0 +
                                (j + v) / strata * info.area->edge1;
          // Shift the light point by an epsilon to avoid being shadowed by any
          // light-related geometry. Since we have the reverse normal, we simply
          // subtract by it.
          light_pos -= kEpsilon * light_reverse_normal;

          glm::vec3 point_to_light = light_pos - shift_pos;
          glm::vec3 light_dir = glm::normalize(point_to_light);
          float light_distance = glm::length(point_to_light);

          // First check to see if light is occluded.
          Ray shadow_ray(shift_pos, light_dir);
          if (scene_.root->HasIntersection(workspace_.get(), shadow_ray,
                                           light_distance)) {
            // Light is occluded; discard the sample.
            continue;
          }

          // Calculate the geometry term, which normalizes for the incident
          // angle on the surface being shaded and for the emission angle from
          // the light source.
          float cos_incident_angle =
              glm::max(glm::dot(hit.normal, light_dir), 0.0f);
          float cos_light_emission_angle =
              glm::max(glm::dot(light_reverse_normal, light_dir), 0.0f);
          float geometry_term = cos_incident_angle * cos_light_emission_angle /
                                (light_distance * light_distance);

          glm::vec3 brdf_term = hit.obj->material->BRDF().Eval(
              light_dir, ray.direction(), hit.normal);

          sample_contributions += geometry_term * brdf_term;
        }
      }
    }

    // Now we must normalize by the light's area and then we can finally
    // compute the final color contribution.
    sample_contributions *= light_area / scene_.light_samples;

    color += info.color * sample_contributions;
  }

  return throughput * color;
}

std::unique_ptr<Integrator> MonteCarloDirect::Clone() const {
  return absl::make_unique<MonteCarloDirect>(*this);
}

glm::vec3 PathTracer::Shade(const Intersection &hit, const Ray &ray,
                            const glm::vec3 &throughput, const int depth) {
  // For physically based rendering, the rendering equation defines the
  // reflected radiance:
  //   L_r(w_o) = L_e(w_o) + ∫_omega(f(w_i, w_o) * L_i(w_i) * (n • w_i) * dw_i)
  // where L_r is the reflected radiance, L_e is the emitted radiance, L_i is
  // the incident radiance along some incident direction w_i, w_o is the
  // outgoing direction, f() is the BRDF, n is the surface normal, and ∫_omega
  // is the integral over all directions in the unit hemisphere.
  //
  // Each step of this integrator represents a single evaluation of the Monte
  // Carlo estimator for the rendering equation, including both direct and
  // indirect terms.
  //
  // In order to improve performance, we can compute the direct lighting at
  // each intersection explicitly from our lighting information, instead of
  // relying on the indirect accumulation via secondary rays that eventually
  // make their way to a light source. This approach is known as next event
  // estimation, and we must avoid double-counting the direct light
  // contribution when we do this.
  //
  // We only trace a single secondary ray at each iteration point because the
  // contribution attenuates after each "bounce" based on the BRDF and cosine
  // terms, meaning that it's more efficient to have many primary rays (since
  // their contribution is strongest) instead of having "bushy" secondary rays
  // per intersection.
  //
  // Further, we can randomly terminate paths with a probability proportional
  // to their current throughput (e.g. how much they'll affect the final
  // color), and then "boost" any paths that survive this test by the inverse
  // of that probability to remain unbiased. This is a technique known as
  // Russian Roulette, and allows us to drop lengthy computations in a deep
  // path which won't contribute much to the final radiance, without
  // introducing bias.
  //

  // Shift the collision point by an epsilon to avoid surfaces shadowing
  // themselves.
  glm::vec3 shift_pos = hit.pos + kEpsilon * hit.normal;

  // Only return indirect light if current depth is being filtered. We only
  // need to check min_depth since max_depth is checked in Integrator::Trace.
  if (depth < scene_.min_depth) {
    return ShadeIndirect(hit, shift_pos, ray, throughput, depth);
  }

  // Only consider emission for base lighting, but take special care when we're
  // using next event estimation. Since emission from the first intersection is
  // not sampled by NEE, we accumulate it _only_ for the first intersection and
  // ignore it for subsequent depths.
  // TODO: We treat emission objects and "lights" a bit differently, and
  // probably incorrectly. Fix this.
  glm::vec3 color;
  // TODO: Is this "reverse tri" check necessary?
  if ((scene_.next_event_estimation && depth > 0) ||
      glm::dot(hit.normal, -ray.direction()) < 0.0f) {
    color = glm::vec3(0.0f);
  } else {
    color = throughput * hit.obj->material->emission;
  }

  if (scene_.next_event_estimation) {
    // Trace the direct light contributions for next event estimation.
    color += ShadeDirect(hit, shift_pos, ray, throughput);
  }

  color += ShadeIndirect(hit, shift_pos, ray, throughput, depth);

  return color;
}

glm::vec3 PathTracer::ShadeIndirect(const Intersection &hit,
                                    const glm::vec3 &shift_pos, const Ray &ray,
                                    const glm::vec3 &throughput,
                                    const int depth) {
  auto &brdf = hit.obj->material->BRDF();

  // First we sample the hemisphere around the surface normal for an outgoing
  // direction.
  glm::vec3 sampled_dir;
  switch (scene_.importance_sampling) {
    case ImportanceSampling::kHemisphere:
      sampled_dir = SampleHemisphere(hit.normal, rand_);
      break;
    case ImportanceSampling::kCosine:
      sampled_dir = SampleCosine(hit.normal, rand_);
      break;
    case ImportanceSampling::kBRDF:
      sampled_dir = brdf.Sample(ray.direction(), hit.normal, rand_);
      break;
  }

  // Early return in case the sample is below the visible hemisphere.
  if (glm::dot(sampled_dir, hit.normal) <= 0.0f) {
    return glm::vec3(0.0f);
  }

  // Compute the BRDF term.
  glm::vec3 brdf_term = brdf.Eval(sampled_dir, ray.direction(), hit.normal);

  // For the final value of the sample, we divide by the sample's probability
  // density function, which in this case equates to a multiply by 2*pi (since
  // there are 2*pi steradians in a hemisphere). The BRDF, cosine term, and
  // sampling PDF together make up the throughput at the current vertex, and we
  // keep track of the current ray's throughput as a recursive product of this,
  // which we pass along to the next indirect ray (note that we don't multiply
  // the result of Trace() with the new throughput, as that would apply it
  // twice).
  // Note that the BRDF and cosine terms suffice to model all
  // physically based attenuation, since radiance does not attenuate with
  // distance.
  glm::vec3 next_throughput;
  switch (scene_.importance_sampling) {
    case ImportanceSampling::kHemisphere:
      // The PDF of hemisphere sampling is just the number of steradians per
      // hemisphere, which is 2pi, so the probability of each sample is 1/2pi,
      // thus the multiplication by 2pi.
      next_throughput = throughput * 2.0f * glm::pi<float>() * brdf_term *
                        glm::max(glm::dot(hit.normal, sampled_dir), 0.0f);
      break;
    case ImportanceSampling::kCosine:
      // The PDF of cosine sampling is (n • w_i) / pi, so the cosine term gets
      // canceled out and we're left with a multiply by pi.
      next_throughput = throughput * glm::pi<float>() * brdf_term;
      break;
    case ImportanceSampling::kBRDF:
      // Estimate the throughput based on the PDF of the current BRDF.
      float pdf = brdf.PDF(sampled_dir, ray.direction(), hit.normal);
      next_throughput = throughput * brdf_term / pdf *
                        glm::max(glm::dot(hit.normal, sampled_dir), 0.0f);
      break;
  }

  // Handle Russian Roulette.
  if (scene_.russian_roulette) {
    // Randomly terminate paths with a probability proportional to their
    // current throughput. This allows us to drop paths that won't be
    // contributing much to the final color anyway.
    float continuation_probability =
        glm::min(glm::compMax(next_throughput), 1.0f);
    if (continuation_probability < rand_.Next()) {
      return glm::vec3(0.0f);
    }

    // Add back the energy we lose by randomly terminating paths, in order to
    // remain unbiased.
    next_throughput /= continuation_probability;
  }

  // Trace the sampled ray.
  Ray sampled_ray(shift_pos, sampled_dir);
  return Trace(sampled_ray, next_throughput, depth + 1);
}

std::unique_ptr<Integrator> PathTracer::Clone() const {
  return absl::make_unique<PathTracer>(*this);
}

}  // namespace muon
