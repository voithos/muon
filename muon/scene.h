#ifndef MUON_SCENE_H_
#define MUON_SCENE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "muon/camera.h"
#include "muon/lighting.h"
#include "muon/objects.h"
#include "muon/types.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// A representation of the scene and its constituents.
// TODO: Currently the scene is also used as a temporary holding area while
// parsing; don't do that.
class Scene {
public:
  // General properties.
  int width = 150;
  int height = 150;
  int max_depth = 5;
  std::string output = "raytrace.png";

  // Lighting properties. Are also copied to scene primitives during parsing.
  glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 ambient = glm::vec3(0.2f);
  glm::vec3 diffuse = glm::vec3(0.0f);
  glm::vec3 specular = glm::vec3(0.0f);
  glm::vec3 emission = glm::vec3(0.0f);
  float shininess = 1.0f;

  std::unique_ptr<Camera> camera;

  void AddVertex(Vertex vert);

  using Primitives = std::vector<std::unique_ptr<Primitive>>;
  using Lights = std::vector<std::unique_ptr<Light>>;

  // Adds a Primitive to the scene, applying the current material defaults.
  void AddPrimitive(std::unique_ptr<Primitive> obj);

  // Adds a Light to the scene, applying the current lighting defaults.
  void AddLight(std::unique_ptr<Light> light);

  inline const std::vector<Vertex> &vertices() const { return vertices_; }

  inline const Primitives &primitives() const { return primitives_; }
  inline const Lights &lights() const { return lights_; }

  // Multiplies the top of the stack with the given transform matrix.
  void MultiplyTransform(const glm::mat4 &m);
  // Pushes the current transform on to the stack.
  void PushTransform();
  // Pops the current transform from the stack.
  void PopTransform();

private:
  std::vector<Vertex> vertices_;
  Primitives primitives_;
  Lights lights_;

  // Transform stack.
  std::vector<glm::mat4> transforms_ = {glm::mat4(1.0f)};
};

} // namespace muon

#endif
