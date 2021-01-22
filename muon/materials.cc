#include "muon/materials.h"

#include <cassert>

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "muon/hemisphere_sampling.h"
#include "muon/transform.h"
#include "third_party/glm/gtc/constants.hpp"
#include "third_party/glm/gtx/component_wise.hpp"

namespace muon {

namespace brdf {

glm::vec3 Phong::Sample(const glm::vec3& ray_dir, const glm::vec3& normal,
                        UniformRandom& rand) {
  assert(material_ != nullptr);

  // First we select between the diffuse and specular lobes.
  float r0 = rand.Next();
  if (r0 > reflectiveness()) {
    // Diffuse sample.
    return SampleCosine(normal, rand);
  }

  // Specular sample.
  // We sample a very sharp lobe and center it on the reflected direction.
  float r1 = rand.Next();
  float r2 = rand.Next();
  float theta = glm::acos(glm::pow(r1, 1.0f / (material_->shininess + 1.0f)));
  float phi = 2.0f * glm::pi<float>() * r2;

  // TODO: Avoid this duplication.
  glm::vec3 s(glm::cos(phi) * glm::sin(theta), glm::sin(phi) * glm::sin(theta),
              glm::cos(theta));

  // Note: this can end up below the visible hemisphere, in which case it
  // should have zero contribution.
  return RotateToOrthonormalFrame(s, Reflect(ray_dir, normal));
}

float Phong::PDF(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                 const glm::vec3& normal) {
  assert(material_ != nullptr);

  // Compute the BRDF's PDF:
  //   pdf(w_i, w_o) =
  //       (1 - t) * (n • w_i) / pi + t * (s + 1) / 2pi * (r • w_i)^s
  // where t is the reflectiveness, w_i is the incident direction, s is the
  // shininess, and r is the reflected vector.
  float t = reflectiveness();
  return (1.0f - t) * glm::max(glm::dot(normal, in_dir), 0.0f) *
             glm::one_over_pi<float>() +
         t * (material_->shininess + 1) * 0.5f * glm::one_over_pi<float>() *
             glm::pow(
                 glm::max(glm::dot(Reflect(ray_dir, normal), in_dir), 0.0f),
                 material_->shininess);
}

glm::vec3 Phong::Eval(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                      const glm::vec3& normal) {
  assert(material_ != nullptr);

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

float Phong::reflectiveness() {
  if (reflectiveness_ < 0.0f) {
    // We average the material's specular and diffuse components to compare the
    // reflectiveness.
    float specular = glm::compAdd(material_->specular) / 3.0f;
    float diffuse = glm::compAdd(material_->diffuse) / 3.0f;
    float denom = diffuse + specular;
    reflectiveness_ = denom > 0.0f ? specular / denom : 0.0f;
  }
  return reflectiveness_;
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
