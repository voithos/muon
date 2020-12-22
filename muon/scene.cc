#include "muon/scene.h"

#include "muon/strings.h"

namespace muon {

void Scene::AddVertex(Vertex vert) { vertices_.push_back(vert); }

void Scene::AddPrimitive(std::unique_ptr<Primitive> obj) {
  primitives_.push_back(std::move(obj));
}

void Scene::AddLight(std::unique_ptr<Light> light) {
  lights_.push_back(std::move(light));
}

} // namespace muon
