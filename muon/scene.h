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
class Scene {
public:
  // General properties.
  int width;
  int height;
  int max_depth;
  std::string output;

  // Global lighting properties.
  glm::vec3 attenuation;

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

private:
  std::vector<Vertex> vertices_;
  Primitives primitives_;
  Lights lights_;
};

} // namespace muon

#endif
