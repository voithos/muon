#ifndef MUON_SCENE_H_
#define MUON_SCENE_H_

#include <string>
#include <vector>

#include "third_party/glm/mat4x4.hpp"
#include "third_party/glm/vec3.hpp"

namespace muon {

class SceneObject {
public:
  glm::mat4 transform;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  glm::vec3 emission;

  float shininess = 0.0f;
};

// A representation of the scene and its constituents.
class Scene {
public:
  // General properties.
  int width = 150;
  int height = 150;
  int max_depth = 5;
  std::string output = "raytrace.png";

  // Lighting properties. Are also copied to scene objects during parsing.
  glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 ambient = glm::vec3(0.2f);
  glm::vec3 diffuse;
  glm::vec3 specular;
  glm::vec3 emission;
  float shininess = 1.0f;

  // Adds a SceneObject to the scene, applying the current lighting defaults.
  void AddObject(SceneObject obj);

private:
  std::vector<SceneObject> objects_;
};

} // namespace muon

#endif
