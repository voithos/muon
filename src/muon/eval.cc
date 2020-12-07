#include "muon/eval.h"

#include <limits>

namespace muon {

glm::vec3 Evaluator::Eval(const Ray &ray) {
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
      hit = intersection;
    }
  }

  // TODO: Add color.
  if (hit) {
    return glm::vec3(1.0, 0, 0);
  }
  return glm::vec3(0.1, 0.1, 0.1);
}

} // namespace muon
