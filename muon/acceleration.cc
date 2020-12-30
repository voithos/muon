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

PrimitiveInfo::PrimitiveInfo(size_t original_index, const Bounds &bounds)
    : original_index(original_index),
      bounds(bounds),
      centroid(glm::vec3(0.0f)) {
  glm::vec3 diagonal = bounds.max_pos - bounds.min_pos;
  centroid = bounds.min_pos + 0.5f * diagonal;
}

BVHNode::BVHNode(size_t num_primitives, size_t start, const Bounds &bounds)
    : num_primitives(num_primitives), start(start), bounds(bounds) {}

BVHNode::BVHNode(std::unique_ptr<BVHNode> left, std::unique_ptr<BVHNode> right,
                 int axis)
    : num_primitives(0),
      axis(axis),
      children{std::move(left), std::move(right)},
      bounds(Bounds::Union(children[0]->bounds, children[1]->bounds)) {}

absl::optional<Intersection> BVH::Intersect(const Ray &ray) {
  // Precompute the child order that we will check for each of the potential
  // split axes based on the sign of the ray in the split axis. If the sign is
  // negative, then we should check the second child since the primitives that
  // went there were of the upper part of the partition point. Note, this
  // doesn't take the ray's origin into account, but for primary rays the
  // assumption is that they will generally originate from "outside" the
  // scene's geometry, and so checking the closer child should allow us to not
  // check too deep into the BVH tree if we've already found a closer match.
  glm::vec3 ray_dir = ray.direction();
  size_t child_to_visit_first[3] = {
      ray_dir.x < 0,
      ray_dir.y < 0,
      ray_dir.z < 0,
  };

  // We perform an iterative depth-first search down the BVH tree, maintaing a
  // stack of nodes to visit and the closest intersection we've seen so far.
  float min_dist = std::numeric_limits<float>::infinity();
  absl::optional<Intersection> hit;
  BVHNode *node = root_.get();

  while (true) {
    // Skip the current node if we don't intersect with its bounds.
    stats_.IncrementBoundsTests();
    if (!node->bounds.HasIntersection(ray, min_dist)) {
      if (frontier_.empty()) {
        break;
      }
      node = frontier_.back();
      frontier_.pop_back();
      continue;
    }
    stats_.IncrementBoundsHits();

    // If this is a leaf node, intersect with the primitives directly.
    if (node->num_primitives > 0) {
      // TODO: De-duplicate this kind of iteration logic.
      for (size_t i = node->start; i < node->start + node->num_primitives;
           ++i) {
        absl::optional<Intersection> intersection =
            primitives_[i]->Intersect(ray);
        stats_.IncrementObjectTests();
        if (!intersection) {
          continue;
        }
        stats_.IncrementObjectHits();

        // Check that the object is in front of the ray's origin, and closer
        // than anything else we've found.
        // TODO: Do we need to check this here? If we add the min_dist argument
        // to the primitives' Intersect method, we could just rely on that.
        if (intersection->distance > 0.0f &&
            intersection->distance < min_dist) {
          min_dist = intersection->distance;
          hit = intersection;
        }
      }
      if (frontier_.empty()) {
        break;
      }
      node = frontier_.back();
      frontier_.pop_back();
      continue;
    }

    // If we reach here, then we're dealing with an internal node.
    size_t c = child_to_visit_first[node->axis];
    // Save the farther child for later.
    frontier_.push_back(node->children[1 - c].get());
    node = node->children[c].get();
  }

  return hit;
}

bool BVH::HasIntersection(const Ray &ray, const float max_distance) {
  // TODO: Make more performant.
  absl::optional<Intersection> hit = Intersect(ray);
  if (!hit) {
    return false;
  }
  // Check that object is in front of the origin, and closer than the target
  // distance.
  return hit->distance > 0.0f && hit->distance < max_distance;
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

  // Recursively build the BVH tree.
  root_ = Build(0, primitives_.size(), primitive_info);

  // Now we must re-order the primitives vector to match the resulting tree's
  // build order, which we can infer from the re-ordered primitive_info vector.
  // We need to do this in order to allow BVHNodes to refer to contiguous
  // blocks of primitives.
  std::vector<std::unique_ptr<Primitive>> sorted_primitives;
  sorted_primitives.reserve(primitives_.size());
  for (const auto &info : primitive_info) {
    sorted_primitives.push_back(std::move(primitives_[info.original_index]));
  }
  primitives_.swap(sorted_primitives);
}

std::unique_ptr<BVHNode> BVH::Build(
    size_t start, size_t end,
    std::vector<PrimitiveInfo> &primitive_info) const {
  size_t size = end - start;
  // Check for base case.
  if (size == 1) {
    const PrimitiveInfo &info = primitive_info[start];
    // Note, we're passing `start` instead of `info.original_index` to the
    // BVHNode because we will later re-build a sorted primitives vector based
    // on the final sort order of primitive_info. This allows us to place
    // primitives in any given BVHNode contiguously in the final primitives
    // vector, which allows us to reference them via a simple index range.
    return absl::make_unique<BVHNode>(size, start, info.bounds);
  }

  // First we figure out the bounds of the centroids in order to choose the
  // axis to split on.
  Bounds centroid_bounds;
  for (size_t i = start; i < end; ++i) {
    centroid_bounds =
        Bounds::Union(centroid_bounds, primitive_info[i].centroid);
  }
  int axis = centroid_bounds.MaxAxis();

  // Now partition the primitives based on the configured partition strategy.
  size_t split;
  auto start_iter = std::next(primitive_info.begin(), start);
  auto end_iter = std::next(primitive_info.begin(), end);

  switch (partition_strategy_) {
    case PartitionStrategy::kMidpoint: {
      // Partition primitives into two, splitting on the midpoint of the
      // centroids.
      float midpoint =
          (centroid_bounds.min_pos[axis] + centroid_bounds.max_pos[axis]) / 2;

      auto split_iter = std::partition(
          start_iter, end_iter, [axis, midpoint](const PrimitiveInfo &info) {
            return info.centroid[axis] < midpoint;
          });
      split = std::distance(primitive_info.begin(), split_iter);
      // If we happen to have failed to partition (e.g. because the primitives
      // have lots of overlapping bounding boxes), then we can simply fall
      // through and have them split uniformly.
      if (split != start && split != end) {
        break;
      }
    }
    case PartitionStrategy::kUniform: {
      split = (start + end) / 2;
      std::nth_element(start_iter, std::next(primitive_info.begin(), split),
                       end_iter,
                       [axis](const PrimitiveInfo &a, const PrimitiveInfo &b) {
                         return a.centroid[axis] < b.centroid[axis];
                       });
    }
  }

  return absl::make_unique<BVHNode>(Build(start, split, primitive_info),
                                    Build(split, end, primitive_info), axis);
}  // namespace acceleration

}  // namespace acceleration
}  // namespace muon
