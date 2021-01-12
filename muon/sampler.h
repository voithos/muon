#ifndef MUON_SAMPLER_H_
#define MUON_SAMPLER_H_

#include <random>

namespace muon {

class Sampler {
 public:
  Sampler(int width, int height, int samples_per_pixel)
      : width_(width),
        height_(height),
        samples_per_pixel_(samples_per_pixel),
        gen_(rd_()),
        rand_(0.0f, 1.0f) {}

  // Generates the next sample location, in terms of x and y coordinates in
  // screen space. If there are no more samples to generate, returns false.
  bool NextSample(float &x, float &y);

  // Returns the total number of samples configured.
  int TotalSamples() const;

  // Returns the number of samples requested so far.
  int RequestedSamples() const;

 private:
  int width_;
  int height_;
  int samples_per_pixel_;
  int cur_x_ = 0;
  int cur_y_ = 0;
  int cur_pixel_sample_ = 0;
  // The number of samples generated so far.
  int samples_ = 0;

  // RNG and seed device for random sampling within the pixel. We use the
  // Mersenne Twister because it has high-quality characteristics.
  // TODO: Replace this and other usages with a central class.
  std::random_device rd_;
  std::mt19937 gen_;
  std::uniform_real_distribution<float> rand_;
};

}  // namespace muon

#endif
