#ifndef MUON_SCENE_H_
#define MUON_SCENE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "muon/camera.h"
#include "third_party/glm/glm.hpp"

namespace muon {

struct Vertex {
  glm::vec3 pos;
};

class SceneObject {
public:
  glm::mat4 transform;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  glm::vec3 emission;

  float shininess = 0.0f;

  // Intersects with a ray and returns the intersection point.
  virtual absl::optional<Intersection> Intersect(const Ray &ray) = 0;
};

class Tri : public SceneObject {
public:
  int v1 = 0;
  int v2 = 0;
  int v3 = 0;

  absl::optional<Intersection> Intersect(const Ray &ray) override;
};

class Sphere : public SceneObject {
public:
  float radius = 0.0f;
  glm::vec3 pos;

  absl::optional<Intersection> Intersect(const Ray &ray) override;
};

// Wrapper around an iterable container to allow a single class to return
// multiple iterable ranges.
template <typename C> class ConstIteratorProxy {
public:
  explicit ConstIteratorProxy(const C &container) : container_(container) {}

  inline typename C::const_iterator begin() const { return container_.begin(); }
  inline typename C::const_iterator end() const { return container_.end(); }

private:
  const C &container_;
};

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

  // Lighting properties. Are also copied to scene objects during parsing.
  glm::vec3 attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 ambient = glm::vec3(0.2f);
  glm::vec3 diffuse;
  glm::vec3 specular;
  glm::vec3 emission;
  float shininess = 1.0f;

  // TODO: Keep track of transforms

  std::unique_ptr<Camera> camera;

  void AddVertex(Vertex vert);

  using SceneObjectList = std::vector<std::unique_ptr<SceneObject>>;

  // Adds a SceneObject to the scene, applying the current lighting defaults.
  void AddObject(std::unique_ptr<SceneObject> obj);

  inline const std::vector<Vertex> &vertices() const { return vertices_; }

  inline const SceneObjectList &meeps() const { return objects_; }

  inline ConstIteratorProxy<SceneObjectList> objects() const {
    return ConstIteratorProxy<SceneObjectList>(objects_);
  }

private:
  std::vector<Vertex> vertices_;
  SceneObjectList objects_;
};

} // namespace muon

#endif
