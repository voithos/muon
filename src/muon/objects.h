#ifndef MUON_OBJECTS_H_
#define MUON_OBJECTS_H_

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "muon/camera.h"
#include "muon/types.h"
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

} // namespace muon

#endif
