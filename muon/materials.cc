#include "muon/materials.h"

#include "third_party/glm/gtc/constants.hpp"

namespace muon {

namespace brdf {

glm::vec3 Phong(const Material& m, const glm::vec3& out_dir,
                const glm::vec3& reflected_dir) {
  // Compute the BRDF. We use a BRDF-compatible version of the Phong reflection
  // model:
  //   f(w_i, w_o) =
  //       (k_d / pi) + (k_s * (s + 2) / (2 * pi)) * (r • w_i)^s
  // where k_d is the diffuse color, k_s is the specular color, s is shininess,
  // and r is the reflection vector.
  glm::vec3 diffuse = m.diffuse * glm::one_over_pi<float>();
  glm::vec3 specular =
      m.specular * (m.shininess + 2.0f) * (0.5f * glm::one_over_pi<float>()) *
      glm::pow(glm::max(glm::dot(reflected_dir, out_dir), 0.0f), m.shininess);
  return diffuse + specular;
}

}  // namespace brdf

}  // namespace muon
