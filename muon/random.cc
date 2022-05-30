#include "muon/random.h"

#include <stdexcept>

#include "glog/logging.h"

namespace muon {

unsigned int SeedGenerator::Next() { return gen_(); }

unsigned int SeedGenerator::GenerateTrueRandomSeed() {
  std::random_device rd_;
  return rd_();
}

float UniformRandom::Next() {
  float r = rand_(gen_);
  if (r == 1.0f) {
    LOG(WARNING) << "Generated a random value outside the expected range! r="
                 << r;
  }
  return r;
}

}  // namespace muon
