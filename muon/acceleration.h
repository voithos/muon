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

// Base scratch space for acceleration structures. Individual acceleration
// structures create their own subtypes.
class Workspace {};

// Abstract base class for acceleration structures. They share similar
// intersection APIs to objects, but sometimes require scratch space.
class Structure {
 public:
  explicit Structure(Stats &stats) : stats_(stats) {}
  virtual ~Structure() = default;

  // Adds a Primitive to the acceleration structure.
  virtual void AddPrimitive(std::unique_ptr<Primitive> obj);

  // Initialize the acceleration structure. Must be called after all primitives
  // have been added.
  virtual void Init() = 0;

  // Creates a unique reusable workspace for the structure. Default
  // implementation returns null, for structures that don't need a workspace.
  virtual std::unique_ptr<Workspace> CreateWorkspace() const { return nullptr; }

  // Intersects with a ray and returns the intersection point. Thread safe as
  // long as each thread has a unique workspace.
  virtual absl::optional<Intersection> Intersect(Workspace *workspace,
                                                 const Ray &ray) const = 0;
  // Returns whether an intersection exists within a distance along the ray.
  // Thread safe as long as each thread has a unique workspace.
  virtual bool HasIntersection(Workspace *workspace, const Ray &ray,
                               const float max_distance) const = 0;

 protected:
  Stats &stats_;
  std::vector<std::unique_ptr<Primitive>> primitives_;
};

// A simple, linear container that intersects all child primitives
// sequentially.
class Linear : public Structure {
 public:
  explicit Linear(Stats &stats) : Structure(stats) {}

  void Init() override {
    // No-op.
  }

  absl::optional<Intersection> Intersect(Workspace *workspace,
                                         const Ray &ray) const override;
  bool HasIntersection(Workspace *workspace, const Ray &ray,
                       const float max_distance) const override;
};

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

// A single node of a BVH tree.
class BVHNode {
 public:
  // Constructs a leaf node.
  BVHNode(size_t num_primitives, size_t start, const Bounds &bounds);
  // Constructs an internal node.
  BVHNode(std::unique_ptr<BVHNode> left, std::unique_ptr<BVHNode> right,
          int axis);

  // The number of primitives in this node. If this is greater than 0, then it
  // is a leaf node; otherwise, it is an internal node.
  const size_t num_primitives;
  // The start primitives index, if this is a leaf node.
  size_t start;
  // The axis that the node is split on, if this is an internal node.
  int axis;
  // The child nodes, if this is an internal node.
  std::array<std::unique_ptr<BVHNode>, 2> children;
  // The bounds of the node.
  const Bounds bounds;
};

constexpr int kBVHStackSize = 64;

// A reusable stack space for use while checking BVH intersection.
class BVHWorkspace : public Workspace {
 public:
  BVHWorkspace() { frontier_.reserve(kBVHStackSize); }

 private:
  // A stack of nodes to visit while checking intersection. Is empty before and
  // after intersection.
  std::vector<BVHNode *> frontier_;

  friend class BVH;
};

// A Bounding Volume Hierarchy that stores primitives based on their proximity.
// This class generates a binary bounding volume hierarchy based on a set of
// primitives, allowing for quick intersection tests. It provides several
// partitioning strategies, including a uniform distribution, centroid midpoint
// split, and splitting based on the surface area heuristic.
class BVH : public Structure {
 public:
  BVH(PartitionStrategy strategy, Stats &stats)
      : Structure(stats), partition_strategy_(strategy) {}

  void Init() override;

  std::unique_ptr<Workspace> CreateWorkspace() const override {
    return absl::make_unique<BVHWorkspace>();
  }

  absl::optional<Intersection> Intersect(Workspace *workspace,
                                         const Ray &ray) const override;
  bool HasIntersection(Workspace *workspace, const Ray &ray,
                       const float max_distance) const override;

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
