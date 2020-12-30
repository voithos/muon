#include "muon/bounds.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "muon/transform.h"

namespace muon {

Bounds::Bounds() {
  // Set the bounds to an invalid state which will still work properly with
  // operations on the bounds.
  min_pos = glm::vec3(std::numeric_limits<float>::infinity());
  max_pos = glm::vec3(-std::numeric_limits<float>::infinity());
}

Bounds::Bounds(const glm::vec3 &pos) {
  min_pos = pos;
  max_pos = pos;
}

Bounds::Bounds(const glm::vec3 &pos1, const glm::vec3 &pos2) {
  min_pos.x = glm::min(pos1.x, pos2.x);
  min_pos.y = glm::min(pos1.y, pos2.y);
  min_pos.z = glm::min(pos1.z, pos2.z);

  max_pos.x = glm::max(pos1.x, pos2.x);
  max_pos.y = glm::max(pos1.y, pos2.y);
  max_pos.z = glm::max(pos1.z, pos2.z);
}

glm::vec3 Bounds::Dimensions() const { return max_pos - min_pos; }

int Bounds::MaxAxis() const {
  glm::vec3 dimensions = Dimensions();
  if (dimensions.x > dimensions.y && dimensions.x > dimensions.z) {
    return 0;  // x
  } else if (dimensions.y > dimensions.z) {
    return 1;  // y
  } else {
    return 2;  // z
  }
}

float Bounds::SurfaceArea() const {
  glm::vec3 dimensions = Dimensions();
  return 2.0f * (dimensions.x * dimensions.y + dimensions.x * dimensions.z +
                 dimensions.y * dimensions.z);
}

Bounds Bounds::Transform(const glm::mat4 &transform) const {
  // Construct a bounding box with all 8 transformed corners of the current
  // bounding box, which will ensure that the new bounds are correct.
  Bounds b(TransformPosition(transform, min_pos));
  b = Union(b, TransformPosition(transform,
                                 glm::vec3(max_pos.x, min_pos.y, min_pos.z)));
  b = Union(b, TransformPosition(transform,
                                 glm::vec3(min_pos.x, max_pos.y, min_pos.z)));
  b = Union(b, TransformPosition(transform,
                                 glm::vec3(min_pos.x, min_pos.y, max_pos.z)));
  b = Union(b, TransformPosition(transform,
                                 glm::vec3(min_pos.x, max_pos.y, max_pos.z)));
  b = Union(b, TransformPosition(transform,
                                 glm::vec3(max_pos.x, min_pos.y, max_pos.z)));
  b = Union(b, TransformPosition(transform,
                                 glm::vec3(max_pos.x, max_pos.y, min_pos.z)));
  return Union(b, TransformPosition(transform, max_pos));
}

bool Bounds::HasIntersection(const Ray &ray) const {
  return HasIntersection(ray, std::numeric_limits<float>::infinity());
}

bool Bounds::HasIntersection(const Ray &ray, const float max_distance) const {
  float t_min, t_max;
  if (!Intersect(ray, t_min, t_max)) {
    return false;
  }
  // Check that the bounds are in front of the origin, and closer than the
  // target distance. We need to be careful with which t value we check: for
  // example, it's possible that t_max > 0 while t_min < 0, in which case the
  // ray origin is "inside" the bounding box and we should count that as a
  // collision.
  return t_max > 0.0f && t_min < max_distance;
}

bool Bounds::Intersect(const Ray &ray, float &t_min, float &t_max) const {
  // Compute the near and far intersection points for each axis, but keep track
  // of a single min/max t value to ensure we get the right intersection point
  // for the shape entry/exit.
  t_min = 0;
  t_max = std::numeric_limits<float>::infinity();
  glm::vec3 origin = ray.origin();
  glm::vec3 direction = ray.direction();

  for (size_t i = 0; i < 3; ++i) {
    // IDEA: This code is called often during BVH traversal, so it may be
    // faster to accept precomputed inverse values for the ray's direction to
    // avoid the relatively costly division.
    float t_axis_min = (min_pos[i] - origin[i]) / direction[i];
    float t_axis_max = (max_pos[i] - origin[i]) / direction[i];

    // The ray isn't necessarily coming from the min->max direction, so swap
    // the t values if needed.
    if (t_axis_min > t_axis_max) {
      std::swap(t_axis_min, t_axis_max);
    }

    // Narrow the t window.
    t_min = glm::max(t_min, t_axis_min);
    t_max = glm::min(t_max, t_axis_max);

    // If we ever reach a point where there is no overlap between our known
    // windows, then we know there's no intersection.
    if (t_min > t_max) {
      return false;
    }
  }

  return true;
}

Bounds Bounds::Union(const Bounds &b1, const Bounds &b2) {
  // Simply compute the minimum of the two bounds' minimums, and the same for
  // the maximum.
  Bounds b;
  b.min_pos.x = glm::min(b1.min_pos.x, b2.min_pos.x);
  b.min_pos.y = glm::min(b1.min_pos.y, b2.min_pos.y);
  b.min_pos.z = glm::min(b1.min_pos.z, b2.min_pos.z);

  b.max_pos.x = glm::max(b1.max_pos.x, b2.max_pos.x);
  b.max_pos.y = glm::max(b1.max_pos.y, b2.max_pos.y);
  b.max_pos.z = glm::max(b1.max_pos.z, b2.max_pos.z);
  return b;
}

Bounds Bounds::Union(const Bounds &b1, const glm::vec3 &pos) {
  Bounds b;
  b.min_pos.x = glm::min(b1.min_pos.x, pos.x);
  b.min_pos.y = glm::min(b1.min_pos.y, pos.y);
  b.min_pos.z = glm::min(b1.min_pos.z, pos.z);

  b.max_pos.x = glm::max(b1.max_pos.x, pos.x);
  b.max_pos.y = glm::max(b1.max_pos.y, pos.y);
  b.max_pos.z = glm::max(b1.max_pos.z, pos.z);
  return b;
}

}  // namespace muon
