#include "muon/acceleration.h"

#include <algorithm>
#include <cassert>
#include <limits>

namespace muon {
namespace acceleration {

void Structure::AddPrimitive(std::unique_ptr<Primitive> obj) {
  primitives_.push_back(std::move(obj));
}

absl::optional<Intersection> Linear::Intersect(Workspace *workspace,
                                               const Ray &ray) const {
  float min_dist = std::numeric_limits<float>::infinity();
  absl::optional<Intersection> hit;

  for (const auto &obj : primitives_) {
    workspace->stats.IncrementObjectTests();
    absl::optional<Intersection> intersection = obj->Intersect(ray);
    if (!intersection) {
      continue;
    }
    workspace->stats.IncrementObjectHits();

    // Check that object is in front of the ray's origin, and closer than
    // anything else we've found.
    if (intersection->distance > 0.0f && intersection->distance < min_dist) {
      min_dist = intersection->distance;
      hit = intersection;
    }
  }
  return hit;
}

bool Linear::HasIntersection(Workspace *workspace, const Ray &ray,
                             const float max_distance) const {
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

absl::optional<Intersection> BVH::Intersect(Workspace *workspace,
                                            const Ray &ray) const {
  std::vector<BVHNode *> &frontier =
      static_cast<BVHWorkspace *>(workspace)->frontier_;
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
    workspace->stats.IncrementBoundsTests();
    if (!node->bounds.HasIntersection(ray, min_dist)) {
      if (frontier.empty()) {
        break;
      }
      node = frontier.back();
      frontier.pop_back();
      continue;
    }
    workspace->stats.IncrementBoundsHits();

    // If this is a leaf node, intersect with the primitives directly.
    if (node->num_primitives > 0) {
      // TODO: De-duplicate this kind of iteration logic.
      for (size_t i = node->start; i < node->start + node->num_primitives;
           ++i) {
        workspace->stats.IncrementObjectTests();
        absl::optional<Intersection> intersection =
            primitives_[i]->Intersect(ray);
        if (!intersection) {
          continue;
        }
        workspace->stats.IncrementObjectHits();

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
      if (frontier.empty()) {
        break;
      }
      node = frontier.back();
      frontier.pop_back();
      continue;
    }

    // If we reach here, then we're dealing with an internal node.
    size_t c = child_to_visit_first[node->axis];
    // Save the farther child for later.
    frontier.push_back(node->children[1 - c].get());
    node = node->children[c].get();
  }

  return hit;
}

bool BVH::HasIntersection(Workspace *workspace, const Ray &ray,
                          const float max_distance) const {
  std::vector<BVHNode *> &frontier =
      static_cast<BVHWorkspace *>(workspace)->frontier_;
  // See Intersect() for details on how the intersection logic works. The main
  // difference here is that we use HasIntersection with the primitives, and
  // return immediately if true.
  glm::vec3 ray_dir = ray.direction();
  size_t child_to_visit_first[3] = {
      ray_dir.x < 0,
      ray_dir.y < 0,
      ray_dir.z < 0,
  };

  BVHNode *node = root_.get();

  while (true) {
    // Skip the current node if we don't intersect with its bounds.
    workspace->stats.IncrementBoundsTests();
    if (!node->bounds.HasIntersection(ray, max_distance)) {
      if (frontier.empty()) {
        break;
      }
      node = frontier.back();
      frontier.pop_back();
      continue;
    }
    workspace->stats.IncrementBoundsHits();

    // If this is a leaf node, intersect with the primitives directly.
    if (node->num_primitives > 0) {
      for (size_t i = node->start; i < node->start + node->num_primitives;
           ++i) {
        workspace->stats.IncrementObjectTests();
        if (primitives_[i]->HasIntersection(ray, max_distance)) {
          workspace->stats.IncrementObjectHits();
          // Clear the frontier since we're exiting before searching it
          // completely.
          frontier.clear();
          return true;
        }
      }
      if (frontier.empty()) {
        break;
      }
      node = frontier.back();
      frontier.pop_back();
      continue;
    }

    // If we reach here, then we're dealing with an internal node.
    size_t c = child_to_visit_first[node->axis];
    // Save the farther child for later.
    frontier.push_back(node->children[1 - c].get());
    node = node->children[c].get();
  }

  return false;
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

// Working info on bucket boundaries while splitting via the surface area
// heuristic.
struct SAHBucketInfo {
  // The number of primitives that currently fall into the bucket.
  size_t size = 0;
  // The composite bounds of all the primitives that are part of this bucket.
  Bounds bounds;
};
constexpr size_t kNumSAHBuckets = 12;

std::unique_ptr<BVHNode> BVH::Build(
    size_t start, size_t end,
    std::vector<PrimitiveInfo> &primitive_info) const {
  assert(start >= 0 && end >= 0);
  size_t num_primitives = end - start;
  // Check for base case.
  if (num_primitives == 1) {
    // Note, we're passing `start` instead of `info.original_index` to the
    // BVHNode because we will later re-build a sorted primitives vector based
    // on the final sort order of primitive_info. This allows us to place
    // primitives in any given BVHNode contiguously in the final primitives
    // vector, which allows us to reference them via a simple index range.
    return absl::make_unique<BVHNode>(num_primitives, start,
                                      primitive_info[start].bounds);
  }

  // First we figure out the bounds of the centroids in order to choose the
  // axis to split on.
  Bounds centroid_bounds;
  for (size_t i = start; i < end; ++i) {
    centroid_bounds =
        Bounds::Union(centroid_bounds, primitive_info[i].centroid);
  }
  // IDEA: Instead of using the widest axis, we could try to consider all axes
  // (e.g. via the surface area heuristic) and pick the one that minimizes
  // cost.
  int axis = centroid_bounds.MaxAxis();

  // Now partition the primitives based on the configured partition strategy.
  size_t split = -1;  // Default to an invalid split point.
  auto start_iter = std::next(primitive_info.begin(), start);
  auto end_iter = std::next(primitive_info.begin(), end);

  bool split_uniformly = false;

  switch (partition_strategy_) {
    case PartitionStrategy::kUniform: {
      split_uniformly = true;
      break;
    }
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
      // have lots of overlapping bounding boxes), then we rely on a uniform
      // split.
      if (split == start || split == end) {
        split_uniformly = true;
      }
      break;
    }
    case PartitionStrategy::kSAH: {
      // Split the primitives by considering several candidate split points and
      // using a surface area heuristic. We do this greedily at each node split
      // point.
      SAHBucketInfo buckets[kNumSAHBuckets];
      glm::vec3 centroid_space = centroid_bounds.Dimensions();

      // Fill the buckets with the working primitives.
      for (size_t i = start; i < end; ++i) {
        // Compute the relative offset of the given primitive's centroid within
        // the space that we're considering splitting. This should always be a
        // number between 0.0 and 1.0.
        // We also need to be careful to avoid division by zero in case
        // primitives are very close together (e.g. have identical centroids).
        float offset = centroid_space[axis] == 0.0f
                           ? 0.0f
                           : (primitive_info[i].centroid[axis] -
                              centroid_bounds.min_pos[axis]) /
                                 centroid_space[axis];
        int bucket_index = static_cast<size_t>(offset * kNumSAHBuckets);
        // Handle edge case for primitive that is at the edge of the
        // centroid_space, in which case we should consider it part of the
        // final bucket.
        if (bucket_index == kNumSAHBuckets) {
          bucket_index = kNumSAHBuckets - 1;
        }
        assert(bucket_index >= 0);
        assert(bucket_index < kNumSAHBuckets);

        SAHBucketInfo &bucket = buckets[bucket_index];
        bucket.size++;
        bucket.bounds = Bounds::Union(bucket.bounds, primitive_info[i].bounds);
      }

      // Also compute the full bucket bounds (i.e. the bounds of all
      // primitives), as we'll need it later.
      Bounds primitive_bounds;
      for (size_t i = 0; i < kNumSAHBuckets; ++i) {
        primitive_bounds = Bounds::Union(primitive_bounds, buckets[i].bounds);
      }

      // Compute the heuristic costs for splitting in between each of the
      // buckets. We don't consider splitting after the last bucket, which
      // wouldn't actually split anything.
      // We arbitrarily define primitive-ray intersections as having a cost of
      // 1, allowing us to simply use the number of primitives in our
      // calculations directly. Then we compute the cost based on the cost of
      // intersecting with each branch of a split (i.e. the number of
      // primitives), times the probability of hitting that branch, which is
      // equivalent to the ratio of the surface area of the branch's bounds
      // divided by the surface area of the parent bounds.
      constexpr int kNumSAHSplitPoints = kNumSAHBuckets - 1;
      float split_costs[kNumSAHSplitPoints];

      // We compute the costs in three passes - first, we compute the partial
      // costs for the left and right branches, and then the final cost while
      // simultaneously determining the min cost.
      {
        size_t combined_size = 0;
        Bounds combined_bounds;
        for (size_t i = 0; i < kNumSAHSplitPoints; ++i) {
          combined_size += buckets[i].size;
          if (combined_size == 0) {
            continue;
          }
          combined_bounds = Bounds::Union(combined_bounds, buckets[i].bounds);
          split_costs[i] = combined_size * combined_bounds.SurfaceArea();
        }
      }
      {
        size_t combined_size = 0;
        Bounds combined_bounds;
        // Need to use a signed integer index since it goes below zero.
        for (int i = kNumSAHSplitPoints - 1; i >= 0; --i) {
          // Use i+1 for the bucket index since we are considering the right
          // branch.
          combined_size += buckets[i + 1].size;
          if (combined_size == 0) {
            continue;
          }
          combined_bounds =
              Bounds::Union(combined_bounds, buckets[i + 1].bounds);
          // TODO: This can sometimes result in a NaN if combined_size is 0 and
          // the surface area is inf.
          split_costs[i] += combined_size * combined_bounds.SurfaceArea();
        }
      }

      float total_surface = primitive_bounds.SurfaceArea();
      // Now we compute the final costs and keep track of the minimum cost we
      // have seen.
      float min_cost = std::numeric_limits<float>::infinity();
      size_t split_bucket;
      for (size_t i = 0; i < kNumSAHSplitPoints; ++i) {
        // The final cost is partial cost normalized by the total surface of
        // the combined bounds, plus a small factor to represent the bounding
        // box intersection cost during rendering.
        constexpr float kBoundingBoxCost = 0.125f;
        float cost = kBoundingBoxCost + split_costs[i] / total_surface;
        if (cost < min_cost) {
          min_cost = cost;
          split_bucket = i;
        }
      }

      // Based on the min cost, we now decide whether to split or to create a
      // leaf node. Since we've defined the cost of intersection with any
      // primitive as 1, the leaf cost is just the number of primitives.
      float leaf_cost = num_primitives;
      if (leaf_cost < min_cost) {
        // See earlier instance of leaf node creation for why we can pass
        // `start` directly here.
        return absl::make_unique<BVHNode>(num_primitives, start,
                                          primitive_bounds);
      }

      auto split_iter = std::partition(
          start_iter, end_iter,
          [axis, split_bucket, &centroid_space,
           &centroid_bounds](const PrimitiveInfo &info) {
            // See earlier bucket computation for details on this partitioning
            // scheme.
            float offset =
                centroid_space[axis] == 0.0f
                    ? 0.0f
                    : (info.centroid[axis] - centroid_bounds.min_pos[axis]) /
                          centroid_space[axis];
            size_t bucket_index = static_cast<size_t>(offset * kNumSAHBuckets);
            if (bucket_index == kNumSAHBuckets) {
              bucket_index = kNumSAHBuckets - 1;
            }
            return bucket_index <= split_bucket;
          });
      split = std::distance(primitive_info.begin(), split_iter);

      // If we happen to have failed to partition (e.g. because the primitives
      // have lots of overlapping bounding boxes), then we rely on a uniform
      // split.
      if (split == start || split == end) {
        split_uniformly = true;
      }
      break;
    }
  }

  // Use uniform split as a fallback.
  if (split_uniformly) {
    // Split primitives uniformly into two groups.
    split = (start + end) / 2;
    std::nth_element(start_iter, std::next(primitive_info.begin(), split),
                     end_iter,
                     [axis](const PrimitiveInfo &a, const PrimitiveInfo &b) {
                       return a.centroid[axis] < b.centroid[axis];
                     });
  }

  return absl::make_unique<BVHNode>(Build(start, split, primitive_info),
                                    Build(split, end, primitive_info), axis);
}  // namespace acceleration

}  // namespace acceleration
}  // namespace muon
