#ifndef MUON_SCENE_H_
#define MUON_SCENE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "muon/acceleration.h"
#include "muon/camera.h"
#include "muon/importance_sampling.h"
#include "muon/lighting.h"
#include "muon/nee.h"
#include "muon/objects.h"
#include "muon/types.h"
#include "muon/vertex.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// A representation of the scene and its constituents.
class Scene {
 public:
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

  Scene() = default;

  // General properties.
  int width;
  int height;
  int min_depth;
  int max_depth;
  std::string output;
  float gamma;
  bool compute_vertex_normals;

  // Integrator properties.
  // TODO: Move these into a separate struct.
  int samples_per_pixel;
  int light_samples;
  bool light_stratify;
  NEE next_event_estimation;
  bool russian_roulette;
  ImportanceSampling importance_sampling;

  // Global lighting properties.
  glm::vec3 attenuation;

  std::unique_ptr<Camera> camera;

  // The root intersectable object for the scene.
  // All tracing starts at this object.
  std::unique_ptr<acceleration::Structure> root;

  void AddVertex(Vertex vert);
  Vertex &GenVertex();

  using Lights = std::vector<std::unique_ptr<Light>>;

  // Adds a Light to the scene.
  void AddLight(std::unique_ptr<Light> light);

  inline std::vector<Vertex> &vertices() { return vertices_; }

  inline Lights &lights() { return lights_; }

 private:
  std::vector<Vertex> vertices_;
  // Vertices that are generated (for e.g. things like lights), as opposed to
  // specified in the scene file.
  // TODO: Is there a better way to do this?
  std::vector<std::unique_ptr<Vertex>> gen_vertices_;
  Lights lights_;
};

}  // namespace muon

#endif
