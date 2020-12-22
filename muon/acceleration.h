#ifndef MUON_ACCELERATION_H_
#define MUON_ACCELERATION_H_

#include <memory>
#include <vector>

#include "absl/types/optional.h"
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
  virtual void AddPrimitive(std::unique_ptr<Primitive> obj) = 0;

protected:
  Stats &stats_;
};

// A simple, linear container that intersects all child primitives
// sequentially.
class Linear : public Structure {
public:
  explicit Linear(Stats &stats) : Structure(stats) {}

  absl::optional<Intersection> Intersect(const Ray &ray) override;
  bool HasIntersection(const Ray &ray, const float max_distance) override;
  void AddPrimitive(std::unique_ptr<Primitive> obj) override;

private:
  std::vector<std::unique_ptr<Primitive>> primitives_;
};

// TODO: Implement.
class BVH : public Structure {
public:
  explicit BVH(Stats &stats) : Structure(stats) {}
};

} // namespace acceleration
} // namespace muon

#endif
