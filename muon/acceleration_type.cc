#include "muon/acceleration_type.h"

namespace muon {

bool AbslParseFlag(absl::string_view text, AccelerationType *type,
                   std::string *error) {
  if (text == "linear") {
    *type = AccelerationType::kLinear;
    return true;
  }
  if (text == "bvh") {
    *type = AccelerationType::kBVH;
    return true;
  }
  *error = "unknown value for acceleration";
  return false;
}

std::string AbslUnparseFlag(AccelerationType type) {
  switch (type) {
    case AccelerationType::kLinear:
      return "linear";
    case AccelerationType::kBVH:
      return "bvh";
    default:
      return absl::StrCat(type);
  }
}

bool AbslParseFlag(absl::string_view text, PartitionStrategy *strategy,
                   std::string *error) {
  if (text == "uniform") {
    *strategy = PartitionStrategy::kUniform;
    return true;
  }
  if (text == "midpoint") {
    *strategy = PartitionStrategy::kMidpoint;
    return true;
  }
  if (text == "sah") {
    *strategy = PartitionStrategy::kSAH;
    return true;
  }
  *error = "unknown value for partition_strategy";
  return false;
}

std::string AbslUnparseFlag(PartitionStrategy strategy) {
  switch (strategy) {
    case PartitionStrategy::kUniform:
      return "uniform";
    case PartitionStrategy::kMidpoint:
      return "midpoint";
    case PartitionStrategy::kSAH:
      return "sah";
    default:
      return absl::StrCat(strategy);
  }
}

}  // namespace muon
