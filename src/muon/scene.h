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
  glm::mat4 inv_transform;
  glm::mat4 inv_transpose_transform;

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
  Tri(const std::vector<Vertex> &vertices, int v0, int v1, int v2);
  absl::optional<Intersection> Intersect(const Ray &ray) override;

private:
  const std::vector<Vertex> &vertices_;
  // Vertices specified in counter-clockwise order.
  int v0_;
  int v1_;
  int v2_;
  // Face normal.
  glm::vec3 normal_;
  // Squared length of the face normal. Used in computing barycentric
  // coordinates.
  float normal_length2_;
};

class Sphere : public SceneObject {
public:
  Sphere(glm::vec3 pos, float radius) : pos_(pos), radius_(radius) {}
  absl::optional<Intersection> Intersect(const Ray &ray) override;

private:
  glm::vec3 pos_;
  float radius_;
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

  std::unique_ptr<Camera> camera;

  void AddVertex(Vertex vert);

  using SceneObjects = std::vector<std::unique_ptr<SceneObject>>;

  // Adds a SceneObject to the scene, applying the current lighting defaults.
  void AddObject(std::unique_ptr<SceneObject> obj);

  inline const std::vector<Vertex> &vertices() const { return vertices_; }

  inline const SceneObjects &objects() const { return objects_; }

  // Multiplies the top of the stack with the given transform matrix.
  void MultiplyTransform(const glm::mat4 &m);
  // Pushes the current transform on to the stack.
  void PushTransform();
  // Pops the current transform from the stack.
  void PopTransform();

private:
  std::vector<Vertex> vertices_;
  SceneObjects objects_;

  // Transform stack.
  std::vector<glm::mat4> transforms_ = {glm::mat4(1.0f)};
};

} // namespace muon

#endif
