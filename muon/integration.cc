#include "muon/integration.h"

#include <limits>

#include "muon/lighting.h"
#include "third_party/glm/gtc/constants.hpp"

namespace muon {

glm::vec3 Integrator::Trace(const Ray &ray) const {
  return Trace(ray, scene_.max_depth);
}

glm::vec3 Integrator::Trace(const Ray &ray, const int depth) const {
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
                           const int depth) const {
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
                                const int depth) const {
  // For physically based rendering, the rendering equation defines the
  // reflected radiance:
  //   L_r(w_o) = L_e(w_o) + ∫_omega(f(w_i, w_o) * L_i(w_i) * (n • w_i) * dw_i)
  // where L_r is the reflected radiance, L_e is the emitted radiance, L_i is
  // the incident radiance along some incident direction w_i, w_o is the
  // outgoing direction, f() is the BRDF, n is the surface normal, and ∫_omega
  // is the integral over all directions in the unit hemisphere.
  //
  // For this integrator, we care only about direct lighting, so we can ignore
  // the emissive term and can also treat the incident radiance L_i as being
  // independent of any direction. We also assume that the shading surface has
  // a Lambertian BRDF which is constant for any w_i and w_o, yielding:
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
    glm::vec3 irradiance_vector;

    for (int i = 0; i < kNumVertices; ++i) {
      int next_i = (i + 1) % kNumVertices;
      glm::vec3 u_i = glm::normalize(vertices[i] - hit.pos);
      glm::vec3 u_next_i = glm::normalize(vertices[next_i] - hit.pos);

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

}  // namespace muon
