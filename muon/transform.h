#ifndef MUON_TRANSFORM_H_
#define MUON_TRANSFORM_H_

#include "third_party/glm/glm.hpp"

namespace muon {

inline glm::vec3 TransformPosition(const glm::mat4 &transform,
                                   const glm::vec3 &pos) {
  return glm::vec3(transform * glm::vec4(pos, 1.0f));
}

inline glm::vec3 TransformDirection(const glm::mat4 &transform,
                                    const glm::vec3 &direction) {
  // Use 0 in the homogeneous coordinate for the direction, since directions
  // shouldn't be translated.
  return glm::normalize(glm::vec3(transform * glm::vec4(direction, 0.0f)));
}

// Reflects an incoming ray direction about the given normal.
// Note that this assumes that the input direction is coming _towards_ the
// surface normal, while the output will be heading _away_.
//
// ray_dir->  ↘ ↑ ↗  <-output
//            -----
inline glm::vec3 Reflect(const glm::vec3 &ray_dir, const glm::vec3 &normal) {
  return ray_dir - (2 * glm::dot(ray_dir, normal) * normal);
}

// Rotates a unit sample vector to an orthonormal coordinate frame constructed
// from vector w, with the original vector's z axis being mapped to w. This is
// used to, for example, rotate an arbitrary hemisphere sample centered about
// the z-axis to be centered about the normal of a surface.
glm::vec3 RotateToOrthonormalFrame(const glm::vec3 &sample, const glm::vec3 &w);

}  // namespace muon

#endif
