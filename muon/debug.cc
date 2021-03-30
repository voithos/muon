#include "muon/debug.h"

#include <cfenv>

#include "absl/flags/flag.h"

ABSL_FLAG(bool, enable_feexcept, false,
          "Whether to enable float point exceptions");

namespace muon {
namespace debug {

void MaybeEnableFloatingPointExceptions() {
  if (absl::GetFlag(FLAGS_enable_feexcept)) {
    feenableexcept(FE_DIVBYZERO | FE_INVALID);
  }
}

}  // namespace debug
}  // namespace muon
