#include "muon/materials.h"

#include "absl/memory/memory.h"
#include "muon/hemisphere_sampling.h"
#include "muon/transform.h"
#include "third_party/glm/gtc/constants.hpp"

namespace muon {

namespace brdf {

glm::vec3 Phong::Sample(const glm::vec3& ray_dir, const glm::vec3& normal,
                        UniformRandom& rand) {
  return SampleHemisphere(normal, rand);
}

float Phong::PDF(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                 const glm::vec3& normal) {
  return 0.5 * glm::one_over_pi<float>();
}

glm::vec3 Phong::Eval(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                      const glm::vec3& normal) {
  // Compute the BRDF. We use a BRDF-compatible version of the Phong reflection
  // model:
  //   f(w_i, w_o) =
  //       (k_d / pi) + (k_s * (s + 2) / (2 * pi)) * (r • w_i)^s
  // where k_d is the diffuse color, k_s is the specular color, s is shininess,
  // and r is the reflection vector.
  glm::vec3 diffuse = material_->diffuse * glm::one_over_pi<float>();
  glm::vec3 specular =
      material_->specular * (material_->shininess + 2.0f) *
      (0.5f * glm::one_over_pi<float>()) *
      glm::pow(glm::max(glm::dot(Reflect(ray_dir, normal), in_dir), 0.0f),
               material_->shininess);
  return diffuse + specular;
}

std::unique_ptr<BRDF> Phong::Clone() const {
  return absl::make_unique<Phong>();
}

}  // namespace brdf

Material::Material(const Material& m) { Copy(m); }

Material& Material::operator=(const Material& m) {
  Copy(m);
  return *this;
}

void Material::Copy(const Material& m) {
  ambient = m.ambient;
  diffuse = m.diffuse;
  specular = m.specular;
  emission = m.emission;

  shininess = m.shininess;

  SetBRDF(m.brdf_->Clone());
}

brdf::BRDF& Material::BRDF() { return *brdf_; }

void Material::SetBRDF(std::unique_ptr<brdf::BRDF> brdf) {
  brdf_ = std::move(brdf);
  brdf_->SetMaterial(this);
}

}  // namespace muon
