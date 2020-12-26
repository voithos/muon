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

}  // namespace muon
