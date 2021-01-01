#include "muon/integration.h"

#include <limits>

#include "muon/lighting.h"

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

}  // namespace muon
