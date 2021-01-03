#ifndef MUON_OBJECTS_H_
#define MUON_OBJECTS_H_

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "muon/bounds.h"
#include "muon/camera.h"
#include "muon/types.h"
#include "third_party/glm/glm.hpp"

namespace muon {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
};

// Represents the material properties of an object.
class Material {
 public:
  glm::vec3 ambient = glm::vec3(0.0f);
  glm::vec3 diffuse = glm::vec3(0.0f);
  glm::vec3 specular = glm::vec3(0.0f);
  glm::vec3 emission = glm::vec3(0.0f);

  float shininess = 0.0f;
};

// Represents an object that supports intersection tests.
class Intersectable {
 public:
  virtual ~Intersectable() {}

  // Intersects with a ray and returns the intersection point.
  virtual absl::optional<Intersection> Intersect(const Ray &ray) = 0;

  // Returns whether an intersection exists within a distance along the ray.
  virtual bool HasIntersection(const Ray &ray, const float max_distance);
};

// Represents a geometric primitive.
// All primitive geometric data is represented in object coordinates, and
// transformed as needed for intersection tests.
class Primitive : public Intersectable {
 public:
  virtual ~Primitive() {}

  // Returns the bounding box that encompasses the geometry of the primitive,
  // in object coordinates.
  virtual Bounds ObjectBounds() const = 0;
  // Returns the bounding box that encompasses the geometry of the primitive,
  // in world coordinates.
  virtual Bounds WorldBounds() const;

  // Transforms the ray to object coordinates and calls IntersectObjectSpace.
  virtual absl::optional<Intersection> Intersect(const Ray &ray) override;

  // Intersects with a ray in object coordinates and returns the intersection
  // point.
  virtual absl::optional<Intersection> IntersectObjectSpace(const Ray &ray) = 0;

  // TODO: Store references to transforms in order to avoid per-primitive
  // duplication.
  glm::mat4 transform;
  glm::mat4 inv_transform;
  glm::mat4 inv_transpose_transform;

  // TODO: Store reference to material to avoid per-primitive duplication.
  Material material;
};

// Represents a triangle.
class Tri : public Primitive {
 public:
  Tri(Vertex &v0, Vertex &v1, Vertex &v2, bool use_vertex_normals);
  // IDEA: Instead of relying on IntersectObjectSpace, tris can pre-transform
  // their vertices once at startup for a speed improvement.
  absl::optional<Intersection> IntersectObjectSpace(const Ray &ray) override;
  Bounds ObjectBounds() const override;
  Bounds WorldBounds() const override;

  glm::vec3 Normal() const { return normal_; }

 private:
  // Vertices specified in counter-clockwise order.
  Vertex &v0_;
  Vertex &v1_;
  Vertex &v2_;
  // Face normal. Note, we store it unnormalized so that we can use it for the
  // intersection calculations.
  glm::vec3 normal_;
  // Squared length of the face normal. Used in computing barycentric
  // coordinates.
  float normal_length2_;
  // Whether or not to use the vertex normals instead of the tri's normal.
  bool use_vertex_normals_;
};

// Represents a sphere.
class Sphere : public Primitive {
 public:
  Sphere(glm::vec3 pos, float radius) : pos_(pos), radius_(radius) {}
  absl::optional<Intersection> IntersectObjectSpace(const Ray &ray) override;
  Bounds ObjectBounds() const override;

 private:
  glm::vec3 pos_;
  float radius_;
};

}  // namespace muon

#endif
