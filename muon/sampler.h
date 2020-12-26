#ifndef MUON_SAMPLER_H_
#define MUON_SAMPLER_H_

namespace muon {

class Sampler {
 public:
  Sampler(int width, int height) : width_(width), height_(height) {}

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
  int cur_x_ = 0;
  int cur_y_ = 0;
  // The number of samples generated so far.
  int samples_ = 0;
};

}  // namespace muon

#endif
