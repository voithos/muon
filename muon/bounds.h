#ifndef MUON_BOUNDS_H_
#define MUON_BOUNDS_H_

#include "muon/ray.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Represents a 3d bounding box.
class Bounds {
 public:
  Bounds();
  Bounds(const glm::vec3 &pos);
  Bounds(const glm::vec3 &pos1, const glm::vec3 &pos2);

  // Returns 0, 1, or 2 to represent x, y, or z, respectively, as the longest
  // axis for the box.
  int MaxAxis() const;

  // Transforms the bounding box by the given transform and returns a new
  // axis-aligned bounding box. Note, because it is axis-aligned, the new
  // bounding box may have different area and volume.
  Bounds Transform(const glm::mat4 &transform) const;

  // Returns whether an intersection exists.
  bool HasIntersection(const Ray &ray) const;
  bool HasIntersection(const Ray &ray, const float max_distance) const;

  // Computes a combined bounding box from an existing bounding box and an
  // additional position.
  static Bounds Union(const Bounds &b1, const glm::vec3 &pos);
  // Computes a combined bounding box from two existing bounding boxes.
  static Bounds Union(const Bounds &b1, const Bounds &b2);

  glm::vec3 min_pos;
  glm::vec3 max_pos;

 private:
  bool Intersect(const Ray &ray, float &t_min, float &t_max) const;
};

}  // namespace muon

#endif
