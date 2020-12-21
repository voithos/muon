#ifndef MUON_TYPES_H_
#define MUON_TYPES_H_

#include "third_party/glm/glm.hpp"

namespace muon {

// Common types and constants shared across muon.

constexpr float kEpsilon = 0.0001;

class Primitive;

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

} // namespace muon

#endif
