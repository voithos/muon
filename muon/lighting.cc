#include "muon/lighting.h"

#include <limits>

namespace muon {

ShadingInfo DirectionalLight::ShadingInfoAt(const glm::vec3 &pos) {
  // Directional lights don't attenuate, and have infinite distance.
  return ShadingInfo{
      .color = color_,
      .direction = direction_,
      .distance = std::numeric_limits<float>::infinity(),
  };
}

ShadingInfo PointLight::ShadingInfoAt(const glm::vec3 &pos) {
  glm::vec3 pos_to_light = this->pos_ - pos;
  glm::vec3 light_dir = glm::normalize(pos_to_light);
  float r = glm::length(pos_to_light);

  // Include the attenuation model (constant, linear, quadratic).
  float attenuation =
      attenuation_.x + attenuation_.y * r + attenuation_.z * r * r;
  glm::vec3 light_color = color_ / attenuation;

  return ShadingInfo{
      .color = light_color,
      .direction = light_dir,
      .distance = r,
  };
}

}  // namespace muon
