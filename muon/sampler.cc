#include "muon/sampler.h"

#include "glog/logging.h"

namespace muon {

bool Sampler::NextSample(float &x, float &y) {
  if (cur_y_ == height_) {
    // If our y index is out of bounds, then we're done.
    return false;
  }

  // Always sample at the center of the pixel for the first sample for backwards
  // compatibility.
  // TODO: Change this to sampling at the center IFF samples_per_pixel_ == 1.
  if (cur_pixel_sample_ == 0) {
    x = cur_x_ + 0.5f;
    y = cur_y_ + 0.5f;
  } else {
    // TODO: This currently samples randomly, but ideally we'd also support
    // stratified sampling.
    float u = rand_.Next();
    float v = rand_.Next();
    x = cur_x_ + u;
    y = cur_y_ + v;

    // At times, we'll overshoot onto the next pixel due to rounding error, so
    // we try to detect this and round to negative infinity if it has occurred.
    if (x == (cur_x_ + 1.0f)) {
      x = std::nextafter(x, -std::numeric_limits<float>::infinity());
    }
    if (y == (cur_y_ + 1.0f)) {
      y = std::nextafter(y, -std::numeric_limits<float>::infinity());
    }
  }

  ++cur_pixel_sample_;
  if (cur_pixel_sample_ == samples_per_pixel_) {
    ++cur_x_;
    cur_pixel_sample_ = 0;
  }
  if (cur_x_ == width_) {
    ++cur_y_;
    cur_x_ = 0;
  }
  ++samples_;
  return true;
}

long int Sampler::TotalSamples() const { return total_samples_; }

long int Sampler::RequestedSamples() const { return samples_; }

float Sampler::Progress() const {
  return samples_ / static_cast<float>(total_samples_);
}

}  // namespace muon
