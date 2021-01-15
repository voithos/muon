#ifndef MUON_RANDOM_H_
#define MUON_RANDOM_H_

#include <random>

namespace muon {

// A random distribution that generates uniform real numbers in the range
// [0.0, 1.0).
class UniformRandom {
 public:
  UniformRandom() : gen_(rd_()), rand_(0.0f, 1.0f) {}
  explicit UniformRandom(unsigned int seed) : gen_(seed), rand_(0.0f, 1.0f) {}

  // Returns the next sampled random number.
  float Next();

 private:
  // RNG and seed device for random sampling. We use the Mersenne Twister
  // because it has high-quality characteristics.
  std::random_device rd_;
  std::mt19937 gen_;
  std::uniform_real_distribution<float> rand_;
};

}  // namespace muon

#endif
