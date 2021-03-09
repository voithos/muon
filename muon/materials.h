#ifndef MUON_MATERIALS_H_
#define MUON_MATERIALS_H_

#include <memory>

#include "muon/random.h"
#include "third_party/glm/glm.hpp"

namespace muon {

class Material;

namespace brdf {

// Base class for all BRDFs.
class BRDF {
 public:
  virtual ~BRDF() {}

  void SetMaterial(Material* material) { material_ = material; }

  // Samples the BRDF, generating a "good" incident ray proportional to the
  // BRDF's probability density function.
  // Note that ray_dir is assumed to be going _toward_ the surface normal.
  virtual glm::vec3 Sample(const glm::vec3& ray_dir, const glm::vec3& normal,
                           UniformRandom& rand) = 0;

  // Returns the result of the probability density function of a given incident
  // ray for the current BRDF.
  // Note that ray_dir is assumed to be going _toward_ the surface normal.
  virtual float PDF(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                    const glm::vec3& normal) = 0;

  // Evaluates the result of the BRDF for the given input parameters.
  // Note that ray_dir is assumed to be going _toward_ the surface normal.
  virtual glm::vec3 Eval(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                         const glm::vec3& normal) = 0;

  // Clones the BRDF implementation.
  virtual std::unique_ptr<BRDF> Clone() const = 0;

 protected:
  Material* material_;
};

// A simple Lambertian reflection model.
//   f(w_i, w_o) = k_d / pi
// where k_d is the diffuse color.
class Lambertian : public BRDF {
 public:
  virtual glm::vec3 Sample(const glm::vec3& ray_dir, const glm::vec3& normal,
                           UniformRandom& rand) override;

  virtual float PDF(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                    const glm::vec3& normal) override;

  virtual glm::vec3 Eval(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                         const glm::vec3& normal) override;

  virtual std::unique_ptr<BRDF> Clone() const override;
};

// A PBR-compatible version of the Phong reflection model.
//   f(w_i, w_o) =
//       (k_d / pi) + (k_s * (s + 2) / (2 * pi)) * (r • w_i)^s
// where k_d is the diffuse color, k_s is the specular color, s is shininess,
// and r is the reflection vector.
class Phong : public BRDF {
 public:
  virtual glm::vec3 Sample(const glm::vec3& ray_dir, const glm::vec3& normal,
                           UniformRandom& rand) override;

  virtual float PDF(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                    const glm::vec3& normal) override;

  virtual glm::vec3 Eval(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                         const glm::vec3& normal) override;

  virtual std::unique_ptr<BRDF> Clone() const override;

 private:
  // Reflectiveness of the material (the amount of "color" that comes from
  // specular component vs combined). This is a number in [0, 1].
  float reflectiveness_ = -1.0f;

  float reflectiveness();
};

// A BRDF based on the GGX microfacet model.
// It consists of a diffuse Lambertian portion and a complex specular component
// defined as:
//   f(w_i, w_o) =
//       F(w_i, h) * G(w_i, w_o) * D(h) /
//       (4 * (w_i • n) * (w_o • n))
// where h is the half vector, F is the fresnel reflection function (which
// accounts for the Fresnel effect), G is the shadowing-masking function (which
// accounts for light bouncing several times on the micro-scale geometry of the
// surface before bouncing away), and D is the microfacet distribution function
// (which defines the probability density of a specific "microsurface normal"
// on the surface).
class GGX : public BRDF {
 public:
  virtual glm::vec3 Sample(const glm::vec3& ray_dir, const glm::vec3& normal,
                           UniformRandom& rand) override;

  virtual float PDF(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                    const glm::vec3& normal) override;

  virtual glm::vec3 Eval(const glm::vec3& in_dir, const glm::vec3& ray_dir,
                         const glm::vec3& normal) override;

  virtual std::unique_ptr<BRDF> Clone() const override;

 private:
  // Reflectiveness of the material (the amount of "color" that comes from
  // specular component vs combined). This is a number in [0, 1].
  float reflectiveness_ = -1.0f;

  float reflectiveness();
};

}  // namespace brdf

// Represents the material properties of an object.
class Material {
 public:
  // TODO: Document these.
  glm::vec3 ambient = glm::vec3(0.0f);
  glm::vec3 diffuse = glm::vec3(0.0f);
  glm::vec3 specular = glm::vec3(0.0f);
  glm::vec3 emission = glm::vec3(0.0f);

  float shininess = 0.0f;
  float roughness = 0.0f;

  Material() {}
  // Allow copying.
  Material(const Material& m);
  Material& operator=(const Material& m);

  brdf::BRDF& BRDF();
  void SetBRDF(std::unique_ptr<brdf::BRDF> brdf);

 private:
  void Copy(const Material& m);

  std::unique_ptr<brdf::BRDF> brdf_;
};

}  // namespace muon

#endif
