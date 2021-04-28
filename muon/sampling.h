#ifndef MUON_SAMPLING_H_
#define MUON_SAMPLING_H_

#include <mutex>
#include <vector>

#include "absl/types/optional.h"
#include "muon/random.h"

namespace muon {

struct Tile {
  // The tile index.
  int idx;
  // The starting x pixel of the tile.
  int x;
  // The starting y pixel of the tile.
  int y;
  // The width of the tile.
  int width;
  // The height of the tile.
  int height;
};

// Returns the recommended number of tiles based on width, height, and the
// number of pixel samples. Having a larger number of tiles allows better use
// of parallelism, but at the cost of some overhead.
int NumTiles(int width, int height, int pixel_samples, int parallelism);

// Generates sub-tiles of an image based on a number of desired tiles.
std::vector<Tile> TileImage(int width, int height, int num_tiles);

// A thread-safe queue of tiles.
class TileQueue {
 public:
  explicit TileQueue(std::vector<Tile> tiles) : tiles_(tiles) {}

  absl::optional<Tile> TryDequeue();

  size_t size() const;

 private:
  std::vector<Tile> tiles_;
  mutable std::mutex mutex_;
};

// A sub-pixel coordinate sampler for sampling the camera plane.
class Sampler {
 public:
  Sampler(Tile tile, int pixel_samples)
      : tile_(tile),
        pixel_samples_(pixel_samples),
        total_samples_(tile.width * tile.height *
                       static_cast<long int>(pixel_samples)) {}

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
  Tile tile_;
  int pixel_samples_;
  long int total_samples_;

  // Current relative x and y positions in the tile.
  int cur_tile_x_ = 0;
  int cur_tile_y_ = 0;
  int cur_pixel_sample_ = 0;
  // The number of samples generated so far.
  long int samples_ = 0;

  // RNG for random sampling within the pixel.
  UniformRandom rand_;
};

}  // namespace muon

#endif
