#ifndef MUON_ACCELERATION_TYPE_H_
#define MUON_ACCELERATION_TYPE_H_

#include <string>

#include "absl/flags/flag.h"

namespace muon {

// The types of acceleration structures available.
enum class AccelerationType {
  // A simple, linear, unaccelerated structure.
  kLinear = 0,
  // A bounding volume hierarchy.
  kBVH,
};

// The strategy used for partitioning primitives within a bounding volume
// hierarchy.
enum class PartitionStrategy {
  // Partition uniformly into two sets.
  kUniform = 0,
  // Partition based on the midpoint of the centroids of the primitives'
  // bounds.
  kMidpoint,
  // Partition based on a surface area heuristic.
  kSAH,
};

bool AbslParseFlag(absl::string_view text, AccelerationType *type,
                   std::string *error);
std::string AbslUnparseFlag(AccelerationType type);

bool AbslParseFlag(absl::string_view text, PartitionStrategy *strategy,
                   std::string *error);
std::string AbslUnparseFlag(PartitionStrategy strategy);

}  // namespace muon

#endif
