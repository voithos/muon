#ifndef MUON_TYPES_H_
#define MUON_TYPES_H_

#include "third_party/glm/glm.hpp"

namespace muon {

// Common types and constants shared across muon.

// TODO: Look into why this needs to be so large to avoid numerical issues.
// TODO: Use a better way of managing intersection error (see
// http://www.pbr-book.org/3ed-2018/Shapes/Managing_Rounding_Error.html).
constexpr float kEpsilon = 0.0001;

class Primitive;

// Represents an intersection of a ray with a primitive.
struct Intersection {
  // Distance along the ray.
  float distance;
  // Intersection point.
  glm::vec3 pos;
  // Normal.
  glm::vec3 normal;
  // Object that we intersected with.
  Primitive *obj;
};

}  // namespace muon

#endif
