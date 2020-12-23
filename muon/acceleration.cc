#include "muon/acceleration.h"

namespace muon {
namespace acceleration {

absl::optional<Intersection> Linear::Intersect(const Ray &ray) {
  float min_dist = std::numeric_limits<float>::infinity();
  absl::optional<Intersection> hit;

  for (const auto &obj : primitives_) {
    absl::optional<Intersection> intersection = obj->Intersect(ray);
    stats_.IncrementObjectTests();
    if (!intersection) {
      continue;
    }
    stats_.IncrementObjectHits();

    // Check that object is in front of the ray's origin, and closer than
    // anything else we've found.
    if (intersection->distance > 0.0f && intersection->distance < min_dist) {
      min_dist = intersection->distance;
      hit = intersection;
    }
  }
  return hit;
}

bool Linear::HasIntersection(const Ray &ray, const float max_distance) {
  for (const auto &obj : primitives_) {
    if (obj->HasIntersection(ray, max_distance)) {
      return true;
    }
  }
  return false;
}

void Linear::AddPrimitive(std::unique_ptr<Primitive> obj) {
  primitives_.push_back(std::move(obj));
}

absl::optional<Intersection> BVH::Intersect(const Ray &ray) {
  return absl::nullopt;
}
bool BVH::HasIntersection(const Ray &ray, const float max_distance) {
  return false;
}
void BVH::AddPrimitive(std::unique_ptr<Primitive> obj) { return; }

} // namespace acceleration
} // namespace muon
