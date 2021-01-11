#include "muon/integration.h"

#include <limits>

#include "glog/logging.h"
#include "muon/lighting.h"
#include "third_party/glm/gtc/constants.hpp"

namespace muon {

glm::vec3 Integrator::Trace(const Ray &ray) {
  return Trace(ray, scene_.max_depth);
}

glm::vec3 Integrator::Trace(const Ray &ray, const int depth) {
  if (depth == 0) {
    return glm::vec3(0.0f);
  }
  absl::optional<Intersection> hit = scene_.root->Intersect(ray);
  if (hit) {
    return Shade(hit.value(), ray, depth);
  }
  return glm::vec3(0.0f);
}

glm::vec3 Raytracer::Shade(const Intersection &hit, const Ray &ray,
                           const int depth) {
  glm::vec3 color = hit.obj->material.ambient + hit.obj->material.emission;

  // Shift the collision point by an epsilon to avoid surfaces shadowing
  // themselves.
  glm::vec3 shift_pos = hit.pos + kEpsilon * hit.normal;

  // Trace the light contributions.
  for (const auto &light : scene_.lights()) {
    ShadingInfo info = light->ShadingInfoAt(hit.pos);
    Ray shadow_ray(shift_pos, info.direction);
    if (scene_.root->HasIntersection(shadow_ray, info.distance)) {
      // Light is occluded.
      continue;
    }
    // Apply a simple Blinn Phong shading model.
    glm::vec3 diffuse_cmp =
        hit.obj->material.diffuse *
        glm::max(glm::dot(hit.normal, info.direction), 0.0f);

    glm::vec3 half_angle = glm::normalize(info.direction - ray.direction());
    glm::vec3 specular_cmp =
        hit.obj->material.specular *
        glm::pow(glm::max(glm::dot(hit.normal, half_angle), 0.0f),
                 hit.obj->material.shininess);

    color += info.color * (diffuse_cmp + specular_cmp);
  }

  // Trace reflectance if the object has any specularity.
  if (glm::any(glm::greaterThan(hit.obj->material.specular, glm::vec3(0.0f)))) {
    glm::vec3 reflected_dir =
        ray.direction() -
        2.0f * glm::dot(hit.normal, ray.direction()) * hit.normal;
    Ray reflected_ray(shift_pos, reflected_dir);

    glm::vec3 reflected_color = Trace(reflected_ray, depth - 1);
    color += hit.obj->material.specular * reflected_color;
  }

  return color;
}

glm::vec3 AnalyticDirect::Shade(const Intersection &hit, const Ray &ray,
                                const int depth) {
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
  glm::vec3 color = hit.obj->material.emission;

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
        hit.obj->material.diffuse * glm::one_over_pi<float>();
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

glm::vec3 MonteCarloDirect::Shade(const Intersection &hit, const Ray &ray,
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
  glm::vec3 color = hit.obj->material.emission;

  // Shift the collision point by an epsilon to avoid surfaces shadowing
  // themselves.
  glm::vec3 shift_pos = hit.pos + kEpsilon * hit.normal;

  glm::vec3 reflected_dir =
      ray.direction() -
      (2 * glm::dot(ray.direction(), hit.normal) * hit.normal);

  // Trace the light contributions.
  for (const auto &light : scene_.lights()) {
    ShadingInfo info = light->ShadingInfoAt(hit.pos);
    // Special case for non-area lights.
    // TODO: This is messy and should probably be refactored. Move the BRDF
    // calculation into the materials?
    // TODO: I'm unconvinced that this is physically accurate...
    if (info.area == nullptr) {
      Ray shadow_ray(shift_pos, info.direction);
      if (scene_.root->HasIntersection(shadow_ray, info.distance)) {
        // Light is occluded.
        continue;
      }
      glm::vec3 irradiance = info.color;
      if (info.distance < std::numeric_limits<float>::infinity()) {
        irradiance /= info.distance * info.distance;
      }
      float cos_incident_angle =
          glm::max(glm::dot(hit.normal, info.direction), 0.0f);
      glm::vec3 phong_brdf =
          brdf::Phong(hit.obj->material, info.direction, reflected_dir);

      color += irradiance * cos_incident_angle * phong_brdf;
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
          float u = rand_(gen_);
          float v = rand_(gen_);
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
          if (scene_.root->HasIntersection(shadow_ray, light_distance)) {
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

          glm::vec3 phong_brdf =
              brdf::Phong(hit.obj->material, light_dir, reflected_dir);

          sample_contributions += geometry_term * phong_brdf;
        }
      }
    }

    // Now we must normalize by the light's area and then we can finally
    // compute the final color contribution.
    sample_contributions *= light_area / scene_.light_samples;

    color += info.color * sample_contributions;
  }

  return color;
}

}  // namespace muon
