#ifndef MUON_LIGHTING_H_
#define MUON_LIGHTING_H_

#include "muon/camera.h"
#include "muon/objects.h"
#include "muon/types.h"
#include "third_party/glm/glm.hpp"

namespace muon {

struct ShadingInfo {
  // Color of the light, taking into account attenuation.
  glm::vec3 color;
  // Direction to the light from the intersection.
  glm::vec3 direction;
  // Distance to the light from the intersection.
  float distance;
};

class Light {
 public:
  explicit Light(glm::vec3 color) : color_(color) {}
  virtual ~Light() {}

  // Calculates information for later shading computation based on an
  // intersection point.
  virtual ShadingInfo ShadingInfoAt(const glm::vec3 &pos) = 0;

  // Samples the shading from the light based on the given shading info.
  virtual glm::vec3 Sample(const ShadingInfo &info, const Intersection &hit,
                           const Ray &ray);

 protected:
  glm::vec3 color_;
};

class DirectionalLight : public Light {
 public:
  DirectionalLight(glm::vec3 color, glm::vec3 direction)
      : Light(color), direction_(direction) {}

  ShadingInfo ShadingInfoAt(const glm::vec3 &pos) override;

 private:
  glm::vec3 direction_;
};

class PointLight : public Light {
 public:
  PointLight(glm::vec3 color, glm::vec3 pos, glm::vec3 attenuation)
      : Light(color), pos_(pos), attenuation_(attenuation) {}

  ShadingInfo ShadingInfoAt(const glm::vec3 &pos) override;

 private:
  glm::vec3 pos_;
  glm::vec3 attenuation_;
};

}  // namespace muon

#endif
