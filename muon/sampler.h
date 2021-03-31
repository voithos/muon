#ifndef MUON_SAMPLER_H_
#define MUON_SAMPLER_H_

#include <vector>

#include "muon/random.h"

namespace muon {

struct Tile {
  // The starting x pixel of the tile.
  int x;
  // The starting y pixel of the tile.
  int y;
  // The width of the tile.
  int width;
  // The height of the tile.
  int height;
};

// Generates sub-tiles of an image based on a number of desired tiles.
std::vector<Tile> TileImage(int width, int height, int num_tiles);

class Sampler {
 public:
  Sampler(int width, int height, int samples_per_pixel)
      : width_(width),
        height_(height),
        samples_per_pixel_(samples_per_pixel),
        total_samples_(width * height *
                       static_cast<long int>(samples_per_pixel)) {}

  // Generates the next sample location, in terms of x and y coordinates in
  // screen space. If there are no more samples to generate, returns false.
  bool NextSample(float &x, float &y);

  // Returns the total number of samples configured.
  long int TotalSamples() const;

  // Returns the number of samples requested so far.
  long int RequestedSamples() const;

  // Returns the rendering progress, based on total and requested sample counts.
  float Progress() const;

 private:
  int width_;
  int height_;
  int samples_per_pixel_;
  long int total_samples_;

  int cur_x_ = 0;
  int cur_y_ = 0;
  int cur_pixel_sample_ = 0;
  // The number of samples generated so far.
  long int samples_ = 0;

  // RNG for random sampling within the pixel.
  UniformRandom rand_;
};

}  // namespace muon

#endif
