#include "muon/lighting.h"

#include <limits>

namespace muon {

absl::optional<Intersection> Quad::Intersect(const Ray &ray) {
  glm::vec3 normal = glm::normalize(glm::cross(edge1, edge0));
  float ray_dot_normal = glm::dot(ray.direction(), normal);
  if (ray_dot_normal == 0.0f) {
    // Ray is parallel to the quad; no intersection.
    return absl::nullopt;
  }
  float t = glm::dot(corner - ray.origin(), normal) / ray_dot_normal;
  if (t < 0) {
    return absl::nullopt;
  }

  glm::vec3 p = ray.At(t);
  glm::vec3 point_edge = p - corner;

  float edge0_length = glm::length(edge0);
  float edge1_length = glm::length(edge1);

  float u = glm::dot(point_edge, edge0) / edge0_length;
  float v = glm::dot(point_edge, edge1) / edge1_length;
  if (u > 0 && u < edge0_length && v > 0 && v < edge1_length) {
    return Intersection{
        .distance = t,
        .pos = p,
        .normal = normal,
        .obj = nullptr,
    };
  }
  return absl::nullopt;
}

ShadingInfo DirectionalLight::ShadingInfoAt(const glm::vec3 &pos) {
  // Directional lights don't attenuate, and have infinite distance.
  return ShadingInfo{
      .color = color_,
      .direction = direction_,
      .distance = std::numeric_limits<float>::infinity(),
      .area = nullptr,
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
      .area = nullptr,
  };
}

ShadingInfo QuadLight::ShadingInfoAt(const glm::vec3 &pos) {
  // For quad lights, the color and area are all that are needed.
  return ShadingInfo{
      .color = color_,
      .area = &area_,
  };
}

}  // namespace muon
