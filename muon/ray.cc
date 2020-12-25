#include "muon/ray.h"

#include "muon/transform.h"

namespace muon {

Ray Ray::Transform(const glm::mat4 &transform) const {
  glm::vec3 t_origin = TransformPosition(transform, origin_);
  glm::vec3 t_direction = TransformDirection(transform, direction_);
  return Ray(t_origin, t_direction);
}

glm::vec3 Ray::At(float t) const { return origin_ + direction_ * t; }

} // namespace muon
