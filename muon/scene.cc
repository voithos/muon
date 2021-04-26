#include "muon/scene.h"

#include "muon/strings.h"

namespace muon {

void Scene::AddVertex(Vertex vert) { meshes_[cur_mesh_idx_].push_back(vert); }

Vertex& Scene::GenVertex() {
  // Create an empty vertex and push it back.
  auto v = absl::make_unique<Vertex>();
  gen_vertices_.push_back(std::move(v));
  return *gen_vertices_.back();
}

void Scene::AddLight(std::unique_ptr<Light> light) {
  lights_.push_back(std::move(light));
}

void Scene::StartMesh() {
  meshes_.push_back(std::vector<Vertex>());
  cur_mesh_idx_ = meshes_.size() - 1;
}

void Scene::EndMesh() { cur_mesh_idx_ = 0; }

}  // namespace muon
