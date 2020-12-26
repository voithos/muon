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

}  // namespace muon

#endif
