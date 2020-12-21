#include "muon/scene.h"

#include "glog/logging.h"
#include "muon/strings.h"

namespace muon {

void Scene::AddVertex(Vertex vert) { vertices_.push_back(vert); }

void Scene::AddPrimitive(std::unique_ptr<Primitive> obj) {
  // Apply current cached lighting.
  obj->ambient = ambient;
  obj->diffuse = diffuse;
  obj->specular = specular;
  obj->emission = emission;
  obj->shininess = shininess;

  obj->transform = transforms_.back();
  obj->inv_transform = glm::inverse(obj->transform);
  obj->inv_transpose_transform = glm::transpose(obj->inv_transform);

  primitives_.push_back(std::move(obj));
}

void Scene::AddLight(std::unique_ptr<Light> light) {
  lights_.push_back(std::move(light));
}

void Scene::MultiplyTransform(const glm::mat4 &m) {
  transforms_.back() = transforms_.back() * m;
  VLOG(3) << "  Current transform: \n" << pprint(transforms_.back());
}

void Scene::PushTransform() {
  transforms_.push_back(transforms_.back());
  VLOG(3) << "  Transform stack size: " << transforms_.size();
  VLOG(3) << "  Current transform: \n" << pprint(transforms_.back());
}

void Scene::PopTransform() {
  transforms_.pop_back();
  VLOG(3) << "  Transform stack size: " << transforms_.size();
}

} // namespace muon
