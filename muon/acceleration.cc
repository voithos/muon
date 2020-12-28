#include "muon/acceleration.h"

#include <algorithm>
#include <limits>

namespace muon {
namespace acceleration {

void Structure::AddPrimitive(std::unique_ptr<Primitive> obj) {
  primitives_.push_back(std::move(obj));
}

absl::optional<Intersection> Linear::Intersect(const Ray &ray) {
  float min_dist = std::numeric_limits<float>::infinity();
  absl::optional<Intersection> hit;

  for (const auto &obj : primitives_) {
    absl::optional<Intersection> intersection = obj->Intersect(ray);
    stats_.IncrementObjectTests();
    if (!intersection) {
      continue;
    }
    stats_.IncrementObjectHits();

    // Check that object is in front of the ray's origin, and closer than
    // anything else we've found.
    if (intersection->distance > 0.0f && intersection->distance < min_dist) {
      min_dist = intersection->distance;
      hit = intersection;
    }
  }
  return hit;
}

bool Linear::HasIntersection(const Ray &ray, const float max_distance) {
  for (const auto &obj : primitives_) {
    if (obj->HasIntersection(ray, max_distance)) {
      return true;
    }
  }
  return false;
}

PrimitiveInfo::PrimitiveInfo(size_t index, const Bounds &bounds)
    : index(index), bounds(bounds), centroid(glm::vec3(0.0f)) {
  glm::vec3 diagonal = bounds.max_pos - bounds.min_pos;
  centroid = bounds.min_pos + 0.5f * diagonal;
}

BVHNode::BVHNode(const std::vector<std::unique_ptr<Primitive>> *primitives,
                 size_t num_primitives, size_t start, const Bounds &bounds,
                 Stats &stats)
    : num_primitives_(num_primitives),
      start_(start),
      primitives_(primitives),
      bounds_(bounds),
      stats_(stats) {}

BVHNode::BVHNode(std::unique_ptr<BVHNode> left, std::unique_ptr<BVHNode> right,
                 int axis, Stats &stats)
    : num_primitives_(0),
      axis_(axis),
      children_{std::move(left), std::move(right)},
      bounds_(Bounds::Union(children_[0]->bounds_, children_[1]->bounds_)),
      stats_(stats) {}

absl::optional<Intersection> BVHNode::Intersect(const Ray &ray) {
  float min_dist = std::numeric_limits<float>::infinity();
  absl::optional<Intersection> hit;

  if (num_primitives_ > 0) {
    // TODO: De-duplicate this kind of iteration logic.
    for (size_t i = start_; i < start_ + num_primitives_; ++i) {
      absl::optional<Intersection> intersection =
          (*primitives_)[i]->Intersect(ray);
      stats_.IncrementObjectTests();
      if (!intersection) {
        continue;
      }
      stats_.IncrementObjectHits();

      // Check that object is in front of the ray's origin, and closer than
      // anything else we've found.
      if (intersection->distance > 0.0f && intersection->distance < min_dist) {
        min_dist = intersection->distance;
        hit = intersection;
      }
    }
    return hit;
  }

  // Check the closer child first, based on the sign of the ray in the split
  // axis. If the sign is negative, then we should check the second child since
  // the primitives that went there were of the upper part of the partition
  // point. Note, this doesn't take the ray's origin into account, but for
  // primary rays the assumption is that they will generally originate from
  // "outside" the scene's geometry, and so checking the closer child should
  // allow us to not check too deep into the BVH tree if we've already found a
  // closer match.
  size_t child_to_check_first = ray.direction()[axis_] < 0;
  for (size_t i = 0; i < 2; ++i) {
    const auto &child = children_[(child_to_check_first + i) % 2];
    if (!child->bounds_.HasIntersection(ray, min_dist)) {
      continue;
    }
    absl::optional<Intersection> intersection = child->Intersect(ray);
    stats_.IncrementBoundsTests();
    if (!intersection) {
      continue;
    }
    stats_.IncrementBoundsHits();

    // Check that object is in front of the ray's origin, and closer than
    // anything else we've found.
    if (intersection->distance > 0.0f && intersection->distance < min_dist) {
      min_dist = intersection->distance;
      hit = intersection;
    }
  }

  return hit;
}

bool BVHNode::HasIntersection(const Ray &ray, const float max_distance) {
  // TODO: Make more performant
  absl::optional<Intersection> hit = Intersect(ray);
  if (!hit) {
    return false;
  }
  // Check that object is in front of the origin, and closer than the target
  // distance.
  return hit->distance > 0.0f && hit->distance < max_distance;
}

absl::optional<Intersection> BVH::Intersect(const Ray &ray) {
  return root_->Intersect(ray);
}

bool BVH::HasIntersection(const Ray &ray, const float max_distance) {
  return root_->HasIntersection(ray, max_distance);
}

void BVH::Init() {
  if (primitives_.empty()) {
    return;
  }

  // Collect object bounds and centroids.
  std::vector<PrimitiveInfo> primitive_info;
  primitive_info.reserve(primitives_.size());
  for (size_t i = 0; i < primitives_.size(); ++i) {
    primitive_info.push_back(PrimitiveInfo(i, primitives_[i]->WorldBounds()));
  }
  root_ = Build(0, primitives_.size(), primitive_info);
}

std::unique_ptr<BVHNode> BVH::Build(
    size_t start, size_t end,
    std::vector<PrimitiveInfo> &primitive_info) const {
  size_t size = end - start;
  // Check for base case.
  if (size == 1) {
    const PrimitiveInfo &info = primitive_info[start];
    return absl::make_unique<BVHNode>(&primitives_, size, info.index,
                                      info.bounds, stats_);
  }

  // First we figure out the bounds of the centroids in order to choose the
  // axis to split on.
  Bounds centroid_bounds;
  for (size_t i = start; i < end; ++i) {
    centroid_bounds =
        Bounds::Union(centroid_bounds, primitive_info[i].centroid);
  }
  int axis = centroid_bounds.MaxAxis();

  // Now partition primitives into two, splitting on the midpoint of the
  // centroids.
  float midpoint =
      (centroid_bounds.min_pos[axis] + centroid_bounds.max_pos[axis]) / 2;

  auto start_iter = std::next(primitive_info.begin(), start);
  auto end_iter = std::next(primitive_info.begin(), end);
  auto split_iter = std::partition(start_iter, end_iter,
                                   [axis, midpoint](const PrimitiveInfo &info) {
                                     return info.centroid[axis] < midpoint;
                                   });
  size_t split = std::distance(primitive_info.begin(), split_iter);

  // If we happen to have failed to partition (e.g. because the primitives have
  // lots of overlapping bounding boxes), then we can simply split uniformly.
  if (split == start || split == end) {
    split = (start + end) / 2;
    std::nth_element(start_iter, std::next(primitive_info.begin(), split),
                     end_iter,
                     [axis](const PrimitiveInfo &a, const PrimitiveInfo &b) {
                       return a.centroid[axis] < b.centroid[axis];
                     });
  }

  return absl::make_unique<BVHNode>(Build(start, split, primitive_info),
                                    Build(split, end, primitive_info), axis,
                                    stats_);
}

}  // namespace acceleration
}  // namespace muon
