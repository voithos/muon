#ifndef MUON_LIGHTING_H_
#define MUON_LIGHTING_H_

#include "third_party/glm/glm.hpp"

namespace muon {

class Light {
public:
  explicit Light(glm::vec3 color) : color_(color) {}

protected:
  glm::vec3 color_;
};

class DirectionalLight : public Light {
public:
  DirectionalLight(glm::vec3 color, glm::vec3 direction)
      : Light(color), direction_(direction) {}

private:
  glm::vec3 direction_;
};

class PointLight : public Light {
public:
  PointLight(glm::vec3 color, glm::vec3 pos, glm::vec3 attenuation)
      : Light(color), pos_(pos), attenuation_(attenuation) {}

private:
  glm::vec3 pos_;
  glm::vec3 attenuation_;
};

} // namespace muon

#endif
