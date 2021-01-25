#include "muon/materials.h"

#include <cassert>

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "muon/hemisphere_sampling.h"
#include "muon/strings.h"
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

// Calculates the GGX microfacet distribution function:
//   D(h) = a^2 / (pi * cos^4(theta_h) * (a^2 + tan^2(theta_h))^2)
float GGXMicrofacetDistribution(float half_angle, float roughness) {
  float a_squared = roughness * roughness;
  float denom =
      glm::pi<float>() * glm::pow(glm::cos(half_angle), 4.0f) *
      glm::pow(a_squared + glm::pow(glm::tan(half_angle), 2.0f), 2.0f);
  assert(denom != 0.0f);
  return a_squared / denom;
}

// Returns a monodirectional shadowing-masking result for the given direction.
//   G_1(v) = 2 / (1 + sqrt(1 + a^2 * tan^2(theta_v)))
// where v is the vector in question and theta_v is the angle between it and
// the normal.
// Note that this should be called for both incident and outgoing directions.
float SmithGGXMonodirectional(const glm::vec3& dir, const glm::vec3& normal,
                              float roughness) {
  float dir_dot_normal = glm::dot(dir, normal);
  if (dir_dot_normal <= 0.0f) {
    return 0.0f;
  }
  float dir_angle = glm::acos(dir_dot_normal);
  float a_squared = roughness * roughness;
  return 2.0f /
         (1.0f +
          glm::sqrt(1.0f + a_squared * glm::pow(glm::tan(dir_angle), 2.0f)));
}

// Returns the shadowing-masking result.
//   G(w_i, w_o) = G_1(w_i) * G_1(w_o)
// where G_1 is the monodirectional shadowing-masking result.
float SmithGGX(const glm::vec3& in_dir, const glm::vec3& ray_dir,
               const glm::vec3& normal, float roughness) {
  // Note that the ray_dir is pointing towards the surface, so we flip it.
  return SmithGGXMonodirectional(in_dir, normal, roughness) *
         SmithGGXMonodirectional(-ray_dir, normal, roughness);
}

// Returns Schlick's approximation of the Fresnel factor.
//   F(w_i, h) = k_s + (1 - k_s) * (1 - (w_i • h))^5
// https://en.wikipedia.org/wiki/Schlick%27s_approximation
glm::vec3 FresnelSchlick(const glm::vec3 in_dir, const glm::vec3& half_vector,
                         const glm::vec3& specular) {
  return specular + (1.0f - specular) *
                        glm::pow(1.0f - glm::dot(in_dir, half_vector), 5.0f);
}

glm::vec3 GGX::Sample(const glm::vec3& ray_dir, const glm::vec3& normal,
                      UniformRandom& rand) {
  assert(material_ != nullptr);

  // First we select between the diffuse and specular lobes.
  float r0 = rand.Next();
  if (r0 > reflectiveness()) {
    // Diffuse sample.
    return SampleCosine(normal, rand);
  }

  // Specular sample.
  // For microfacet BRDFs, instead of sampling the target lobe directly, we
  // instead generate a half vector from the microfacet distribution.
  float r1 = rand.Next();
  float r2 = rand.Next();
  float theta = glm::atan(material_->roughness * glm::sqrt(r1) *
                          glm::inversesqrt(1.0f - r1));
  float phi = 2.0f * glm::pi<float>() * r2;

  // TODO: Avoid this duplication.
  glm::vec3 s(glm::cos(phi) * glm::sin(theta), glm::sin(phi) * glm::sin(theta),
              glm::cos(theta));

  glm::vec3 half_vector = RotateToOrthonormalFrame(s, normal);

  // Finally, we must reflect the ray direction over the half vector to obtain
  // the final sample.
  return Reflect(ray_dir, half_vector);
}

float GGX::PDF(const glm::vec3& in_dir, const glm::vec3& ray_dir,
               const glm::vec3& normal) {
  assert(material_ != nullptr);

  // Compute the BRDF's PDF:
  //   pdf(w_i, w_o) =
  //       (1 - t) * (n • w_i) / pi + t * D(h) * (n • h) / (4 * (h • w_i))
  // where t is the reflectiveness, w_i is the incident direction, and h is the
  // half vector.
  float t = reflectiveness();
  glm::vec3 half_vector = glm::normalize(in_dir - ray_dir);
  float half_angle = glm::acos(glm::dot(half_vector, normal));
  return (1.0f - t) * glm::max(glm::dot(normal, in_dir), 0.0f) *
             glm::one_over_pi<float>() +
         t * GGXMicrofacetDistribution(half_angle, material_->roughness) *
             glm::max(glm::dot(normal, half_vector), 0.0f) /
             (4.0f * glm::dot(half_vector, in_dir));
}

glm::vec3 GGX::Eval(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                    const glm::vec3& normal) {
  assert(material_ != nullptr);

  // Compute the BRDF:
  //   f(w_i, w_o) =
  //       F(w_i, h) * G(w_i, w_o) * D(h) /
  //       (4 * (w_i • n) * (w_o • n))
  // where h is the half vector, F is the fresnel reflection function (which
  // accounts for the Fresnel effect), G is the shadowing-masking function
  // (which accounts for light bouncing several times on the micro-scale
  // geometry of the surface before bouncing away), and D is the microfacet
  // distribution function (which defines the probability density of a specific
  // "microsurface normal" on the surface).

  // First we define some reused values.
  glm::vec3 half_vector = glm::normalize(in_dir - ray_dir);
  float half_angle = glm::acos(glm::dot(half_vector, normal));

  // We compute the diffuse component as a standard Lambertian.
  glm::vec3 diffuse = material_->diffuse * glm::one_over_pi<float>();

  // Finally, for the specular term, we evaluate GGX.
  glm::vec3 fresnel = FresnelSchlick(in_dir, half_vector, material_->specular);
  float shadowing_masking =
      SmithGGX(in_dir, ray_dir, normal, material_->roughness);
  float microfacet_distribution =
      GGXMicrofacetDistribution(half_angle, material_->roughness);

  // The ray_dir is facing towards the surface, so we flip it when computing the
  // dot product with the normal.
  glm::vec3 specular =
      fresnel * shadowing_masking * microfacet_distribution /
      (4.0f * glm::dot(in_dir, normal) * glm::dot(-ray_dir, normal));

  return diffuse + specular;
}

std::unique_ptr<BRDF> GGX::Clone() const { return absl::make_unique<GGX>(); }

float GGX::reflectiveness() {
  if (reflectiveness_ < 0.0f) {
    // This is almost identical to the reflectiveness calculation we use in the
    // Phong BRDF, except we always retain a minimal reflectiveness value even
    // for completely diffuse surfaces in order to sample for the Fresnel
    // effect.
    float specular = glm::compAdd(material_->specular) / 3.0f;
    float diffuse = glm::compAdd(material_->diffuse) / 3.0f;
    float denom = diffuse + specular;
    if (denom == 0.0f) {
      // If both diffuse and specular components of the material are zero, then
      // we only really care about the Fresnel effect, so we should just always
      // the specular lobe.
      reflectiveness_ = 1.0f;
    } else {
      // Otherwise we still want to sample the specular lobe reasonably
      // frequently in order to account for the Fresnel effect, so retain a
      // minimum reflectiveness.
      // TODO: Move this magic number somewhere else.
      reflectiveness_ = glm::max(specular / denom, 0.25f);
    }
  }
  return reflectiveness_;
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
