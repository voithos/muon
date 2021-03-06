#include "muon/transform.h"

namespace muon {

glm::vec3 RotateToOrthonormalFrame(const glm::vec3& sample,
                                   const glm::vec3& w) {
  // We rotate by constructing an orthonormal coordinate frame around the
  // target vector and projecting the sample onto it.
  // Arbitrarily use the up vector to generate the coordinate frame, except if
  // the target vector is too close to it. We check the abs(y) since a vector
  // in the negative y direction would have the same issue.
  glm::vec3 a = glm::abs(w.y) <= 0.9f ? glm::vec3(0.0f, 1.0f, 0.0f)
                                      : glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 u = glm::normalize(glm::cross(a, w));
  glm::vec3 v = glm::normalize(glm::cross(w, u));

  // Now we rotate the sample to find the final sample.
  return sample.x * u + sample.y * v + sample.z * w;
}

}  // namespace muon
