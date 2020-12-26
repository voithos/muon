#include "muon/lighting.h"

#include <limits>

namespace muon {

glm::vec3 Light::Sample(const ShadingInfo &info, const Intersection &hit,
                        const Ray &ray) {
  // Apply a simple Blinn Phong shading model.
  glm::vec3 diffuse_cmp = hit.obj->material.diffuse *
                          glm::max(glm::dot(hit.normal, info.direction), 0.0f);

  glm::vec3 half_angle = glm::normalize(info.direction - ray.direction());
  glm::vec3 specular_cmp =
      hit.obj->material.specular *
      glm::pow(glm::max(glm::dot(hit.normal, half_angle), 0.0f),
               hit.obj->material.shininess);

  return info.color * (diffuse_cmp + specular_cmp);
}

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
