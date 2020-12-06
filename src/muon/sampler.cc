#include "muon/sampler.h"

namespace muon {

bool Sampler::NextSample(float &x, float &y) {
  // Currently just generates a sample per pixel, at the midpoint.
  if (cur_y_ == height_) {
    // If our y index is out of bounds, then we're done.
    return false;
  }
  x = cur_x_ + 0.5f;
  y = cur_y_ + 0.5f;
  ++cur_x_;
  if (cur_x_ == width_) {
    ++cur_y_;
    cur_x_ = 0;
  }
  return true;
}

} // namespace muon
