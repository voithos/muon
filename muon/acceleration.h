#ifndef MUON_ACCELERATION_H_
#define MUON_ACCELERATION_H_

#include <array>
#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "muon/acceleration_type.h"
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
// Working info on primitives used during BVH construction.
class PrimitiveInfo {
 public:
  PrimitiveInfo(size_t original_index, const Bounds &bounds);

  // The original index into the primitives vector. After the BVH is built, we
  // use the original index to sort the primitives vector to match the build
  // order.
  size_t original_index;
  // The cached world-space bounds of the primitive.
  Bounds bounds;
  // The centroid of the primitive bounds.
  glm::vec3 centroid;
};
}  // namespace

// A single node of a BVH tree.
class BVHNode {
 public:
  // Constructs a leaf node.
  BVHNode(const std::vector<std::unique_ptr<Primitive>> *primitives,
          size_t num_primitives, size_t start, const Bounds &bounds,
          Stats &stats);
  // Constructs an internal node.
  BVHNode(std::unique_ptr<BVHNode> left, std::unique_ptr<BVHNode> right,
          int axis, Stats &stats);

  absl::optional<Intersection> Intersect(const Ray &ray);
  bool HasIntersection(const Ray &ray, const float max_distance);

 private:
  // The number of primitives in this node. If this is greater than 0, then it
  // is a leaf node; otherwise, it is an internal node.
  const size_t num_primitives_;
  // The start primitives index, if this is a leaf node.
  size_t start_;
  // The axis that the node is split on, if this is an internal node.
  int axis_;
  // The primitives collection.
  const std::vector<std::unique_ptr<Primitive>> *primitives_;
  // The child nodes, if this is an internal node.
  std::array<std::unique_ptr<BVHNode>, 2> children_;
  // The bounds of the node.
  const Bounds bounds_;
  // The stats tracking.
  Stats &stats_;
};

// A Bounding Volume Hierarchy that stores primitives based on their proximity.
// TODO: Describe
class BVH : public Structure {
 public:
  BVH(PartitionStrategy strategy, Stats &stats)
      : Structure(stats), partition_strategy_(strategy) {}

  absl::optional<Intersection> Intersect(const Ray &ray) override;
  bool HasIntersection(const Ray &ray, const float max_distance) override;
  void Init() override;

 private:
  PartitionStrategy partition_strategy_;
  std::unique_ptr<BVHNode> root_;

  // Recursively builds the BVH tree out of a given start and end range in the
  // primitives vector.
  std::unique_ptr<BVHNode> Build(size_t start, size_t end,
                                 std::vector<PrimitiveInfo> &info) const;
};

}  // namespace acceleration
}  // namespace muon

#endif
