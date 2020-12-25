#ifndef MUON_ACCELERATION_H_
#define MUON_ACCELERATION_H_

#include <array>
#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "muon/bounds.h"
#include "muon/objects.h"
#include "muon/stats.h"

namespace muon {
namespace acceleration {

// Abstract base class for acceleration structures.
class Structure : public Intersectable {
public:
  explicit Structure(Stats &stats) : stats_(stats) {}
  virtual ~Structure() {}

  // Adds a Primitive to the acceleration structure.
  virtual void AddPrimitive(std::unique_ptr<Primitive> obj);

  // Initialize the acceleration structure. Must be called after all primitives
  // have been added.
  virtual void Init() = 0;

protected:
  Stats &stats_;
  std::vector<std::unique_ptr<Primitive>> primitives_;
};

// A simple, linear container that intersects all child primitives
// sequentially.
class Linear : public Structure {
public:
  explicit Linear(Stats &stats) : Structure(stats) {}

  absl::optional<Intersection> Intersect(const Ray &ray) override;
  bool HasIntersection(const Ray &ray, const float max_distance) override;
  void Init() override {
    // No-op.
  }
};

namespace {
// Working info on primitives used during BVH creation.
class PrimitiveInfo {
public:
  PrimitiveInfo(size_t index, const Bounds &bounds);

  size_t index;
  Bounds bounds;
  glm::vec3 centroid;
};
} // namespace

// A single node of a BVH tree.
class BVHNode {
public:
  // Constructs a leaf node.
  BVHNode(const std::vector<std::unique_ptr<Primitive>> *primitives,
          size_t start, size_t end, const Bounds &bounds, Stats &stats);
  // Constructs an internal node.
  BVHNode(std::unique_ptr<BVHNode> left, std::unique_ptr<BVHNode> right,
          Stats &stats);

  absl::optional<Intersection> Intersect(const Ray &ray);
  bool HasIntersection(const Ray &ray, const float max_distance);

private:
  // Whether or not this node is a leaf node.
  const bool is_leaf_;
  // The primitives collection.
  const std::vector<std::unique_ptr<Primitive>> *primitives_;
  // The start and (exclusive) end primitives indices.
  size_t start_, end_;
  // The child nodes.
  std::array<std::unique_ptr<BVHNode>, 2> children_;
  // The bounds of the node.
  const Bounds bounds_;
  // The statis tracking.
  Stats &stats_;
};

// A Bounding Volume Hierarchy that stores primitives based on their proximity.
// TODO: Describe
class BVH : public Structure {
public:
  BVH(Stats &stats) : Structure(stats) {}

  absl::optional<Intersection> Intersect(const Ray &ray) override;
  bool HasIntersection(const Ray &ray, const float max_distance) override;
  void Init() override;

private:
  std::unique_ptr<BVHNode> root_;

  // Recursively builds the BVH tree out of a given start and end range in the
  // primitives vector.
  std::unique_ptr<BVHNode> Build(size_t start, size_t end,
                                 std::vector<PrimitiveInfo> &info) const;
};

} // namespace acceleration
} // namespace muon

#endif
