#ifndef MUON_RAY_H_
#define MUON_RAY_H_

#include "third_party/glm/glm.hpp"

namespace muon {

class Ray {
public:
  Ray(const glm::vec3 &origin, const glm::vec3 &direction)
      : origin_(origin), direction_(direction) {}
  glm::vec3 origin() const { return origin_; }
  glm::vec3 direction() const { return direction_; }

  // Transforms the ray using the given transformation matrix, returning a new
  // ray.
  Ray Transform(const glm::mat4 &transform) const;

  // Returns a point on the ray at a given "distance" t.
  glm::vec3 At(float t) const;

private:
  glm::vec3 origin_;
  glm::vec3 direction_;
};

} // namespace muon

#endif
