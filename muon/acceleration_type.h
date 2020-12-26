#ifndef MUON_ACCELERATION_TYPE_H_
#define MUON_ACCELERATION_TYPE_H_

#include <string>

#include "absl/flags/flag.h"

namespace muon {

// The types of acceleration structures available.
enum class AccelerationType {
  kLinear = 0,
  kBVH,
};

bool AbslParseFlag(absl::string_view text, AccelerationType *type,
                   std::string *error);

std::string AbslUnparseFlag(AccelerationType type);

}  // namespace muon

#endif
