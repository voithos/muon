#include "muon/eval.h"

#include <limits>

namespace muon {

glm::vec3 Evaluator::Eval(const Ray &ray) {
  float min_dist = std::numeric_limits<float>::infinity();
  absl::optional<Intersection> hit;
  std::shared_ptr<SceneObject> hit_obj;

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
      hit_obj = obj;
    }
  }

  // TODO: Add color.
  if (hit) {
    return hit_obj->ambient + hit_obj->emission;
  }
  return glm::vec3(0.0f);
}

} // namespace muon
