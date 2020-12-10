#ifndef MUON_TYPES_H_
#define MUON_TYPES_H_

#include "third_party/glm/glm.hpp"

namespace muon {

// Common types and constants shared across muon.

constexpr float kEpsilon = 0.0001;

class SceneObject;

struct Intersection {
  // Distance along the ray.
  float distance;
  // Intersection point.
  glm::vec3 pos;
  // Normal.
  glm::vec3 normal;
  // Object that we intersected with.
  SceneObject *obj;
};

} // namespace muon

#endif
