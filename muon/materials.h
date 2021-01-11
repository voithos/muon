#ifndef MUON_MATERIALS_H_
#define MUON_MATERIALS_H_

#include "third_party/glm/glm.hpp"

namespace muon {

// Represents the material properties of an object.
class Material {
 public:
  glm::vec3 ambient = glm::vec3(0.0f);
  glm::vec3 diffuse = glm::vec3(0.0f);
  glm::vec3 specular = glm::vec3(0.0f);
  glm::vec3 emission = glm::vec3(0.0f);

  float shininess = 0.0f;
};

// TODO: Create subclass materials instead of having separate functions for the
// BRDFs.
namespace brdf {

// Computes a BRDF-compatible version of the Phong reflection model.
//   f(w_i, w_o) =
//       (k_d / pi) + (k_s * (s + 2) / (2 * pi)) * (r • w_i)^s
// where k_d is the diffuse color, k_s is the specular color, s is shininess,
// and r is the reflection vector.
glm::vec3 Phong(const Material& m, const glm::vec3& out_dir,
                const glm::vec3& reflected_dir);

}  // namespace brdf

}  // namespace muon

#endif
