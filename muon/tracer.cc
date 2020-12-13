#include "muon/tracer.h"

#include <limits>

#include "muon/lighting.h"

namespace muon {

glm::vec3 Tracer::Trace(const Ray &ray) const {
  return Trace(ray, scene_.max_depth);
}

glm::vec3 Tracer::Trace(const Ray &ray, const int depth) const {
  if (depth == 0) {
    return glm::vec3(0.0f);
  }

  float min_dist = std::numeric_limits<float>::infinity();
  absl::optional<Intersection> hit;

  for (const auto &obj : scene_.objects()) {
    absl::optional<Intersection> intersection = obj->Intersect(ray);
    if (!intersection) {
      continue;
    }
    // Check that object is in front of the camera, and closer than anything
    // else we've found.
    if (intersection->distance > 0.0f && intersection->distance < min_dist) {
      min_dist = intersection->distance;
      hit = intersection;
    }
  }

  if (hit) {
    return Shade(hit.value(), ray, depth);
  }
  return glm::vec3(0.0f);
}

glm::vec3 Tracer::Shade(const Intersection &hit, const Ray &ray,
                        const int depth) const {
  glm::vec3 color = hit.obj->ambient + hit.obj->emission;

  // Shift the collision point by an epsilon to avoid surfaces shadowing
  // themselves.
  glm::vec3 shift_pos = hit.pos + kEpsilon * hit.normal;

  // Trace the light contributions.
  for (const auto &light : scene_.lights()) {
    ShadingInfo info = light->ShadingInfoAt(hit.pos);
    Ray shadow_ray(shift_pos, info.direction);
    if (IsOccluded(shadow_ray, info.distance)) {
      // Light is occluded.
      continue;
    }
    color += light->Sample(info, hit, ray);
  }

  // Trace reflectance if the object has any specularity.
  if (glm::any(glm::greaterThan(hit.obj->specular, glm::vec3(0.0f)))) {
    glm::vec3 reflected_dir =
        ray.direction() -
        2.0f * glm::dot(hit.normal, ray.direction()) * hit.normal;
    Ray reflected_ray(shift_pos, reflected_dir);

    glm::vec3 reflected_color = Trace(reflected_ray, depth - 1);
    color += hit.obj->specular * reflected_color;
  }

  return color;
}

bool Tracer::IsOccluded(const Ray &shadow_ray,
                        const float light_distance) const {
  for (const auto &obj : scene_.objects()) {
    absl::optional<Intersection> intersection = obj->Intersect(shadow_ray);
    if (!intersection) {
      continue;
    }
    // Check that object is in front of the origin, and closer than the target
    // distance (meaning that it is occluding us).
    if (intersection->distance > 0.0f &&
        intersection->distance < light_distance) {
      return true;
    }
  }
  return false;
}

} // namespace muon
