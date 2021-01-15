#include "muon/random.h"

#include <stdexcept>

#include "glog/logging.h"

namespace muon {

float UniformRandom::Next() {
  float r = rand_(gen_);
  if (r == 1.0f) {
    LOG(WARNING) << "Generated a random value outside the expected range! r="
                 << r;
  }
  return r;
}

}  // namespace muon
