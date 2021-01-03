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
  bool compute_vertex_normals;

  // Integrator properties.
  int light_samples;
  bool light_stratify;

  // Global lighting properties.
  glm::vec3 attenuation;

  std::unique_ptr<Camera> camera;

  // The root intersectable object for the scene.
  // All tracing starts at this object.
  std::unique_ptr<Intersectable> root;

  void AddVertex(Vertex vert);

  using Lights = std::vector<std::unique_ptr<Light>>;

  // Adds a Light to the scene.
  void AddLight(std::unique_ptr<Light> light);

  inline std::vector<Vertex> &vertices() { return vertices_; }

  inline Lights &lights() { return lights_; }

 private:
  std::vector<Vertex> vertices_;
  Lights lights_;
};

}  // namespace muon

#endif
