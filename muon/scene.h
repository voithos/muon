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
  std::unique_ptr<SeedGenerator> seedgen;  // Random seed generation.

  int width;
  int height;
  int min_depth;
  int max_depth;
  std::string output;
  float gamma;
  bool compute_vertex_normals;

  // Integrator properties.
  // TODO: Move these into a separate struct.
  int pixel_samples;
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

  // Adds a vertex to the current mesh.
  void AddVertex(Vertex vert);

  // Adds a vertex to the set of "internal", generated vertices.
  Vertex &GenVertex();

  using Lights = std::vector<std::unique_ptr<Light>>;

  // Adds a Light to the scene.
  void AddLight(std::unique_ptr<Light> light);

  // Starts a new mesh and switches context to it.
  void StartMesh();

  // Ends the current mesh and defaults back to the global one.
  void EndMesh();

  // Returns the set of all meshes.
  inline std::vector<std::vector<Vertex>> &meshes() { return meshes_; }

  // Returns the vertices for the current mesh.
  inline std::vector<Vertex> &vertices() { return meshes_[cur_mesh_idx_]; }

  // Returns the set of all lights.
  inline Lights &lights() { return lights_; }

 private:
  // The set of meshes comprising the current scene. Initially just a single
  // global mesh.
  std::vector<std::vector<Vertex>> meshes_ = {{}};
  int cur_mesh_idx_ = 0;

  // Vertices that are generated (for e.g. things like lights), as opposed to
  // specified in the scene file.
  // TODO: Is there a better way to do this?
  std::vector<std::unique_ptr<Vertex>> gen_vertices_;
  Lights lights_;
};

}  // namespace muon

#endif
