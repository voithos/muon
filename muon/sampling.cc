#include "muon/sampling.h"

#include <cassert>

#include "glog/logging.h"

namespace muon {

std::vector<Tile> TileImage(int width, int height, int num_tiles) {
  // For now, we just split the image vertically into thin "tiles".
  // In the future, this could do something more interesting / heuristic.
  std::vector<Tile> tiles;
  tiles.reserve(num_tiles);

  int tile_height = height / num_tiles;
  assert(tile_height > 0);

  // Account for leftover due to integer truncation.
  int excess_height = height - tile_height * num_tiles;
  Tile first_tile = {.idx = 0,
                     .x = 0,
                     .y = 0,
                     .width = width,
                     .height = tile_height + excess_height};
  assert(first_tile.x + first_tile.width <= width);
  assert(first_tile.y + first_tile.height <= height);
  tiles.push_back(first_tile);

  // Generate subsequent tiles.
  for (int i = 1; i < num_tiles; ++i) {
    Tile tile = {.idx = i,
                 .x = 0,
                 .y = i * tile_height + excess_height,
                 .width = width,
                 .height = tile_height};
    assert(tile.x + tile.width <= width);
    assert(tile.y + tile.height <= height);
    tiles.push_back(tile);
  }
  return tiles;
}

absl::optional<Tile> TileQueue::TryDequeue() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (tiles_.empty()) {
    return absl::nullopt;
  }
  Tile t = tiles_.back();
  tiles_.pop_back();
  return t;
}

bool Sampler::NextSample(float &x, float &y) {
  if (cur_tile_y_ == tile_.height) {
    // If our y index is out of bounds, then we're done.
    return false;
  }

  // Always sample at the center of the pixel for the first sample for backwards
  // compatibility.
  // TODO: Change this to sampling at the center IFF pixel_samples_ == 1.
  if (cur_pixel_sample_ == 0) {
    x = tile_.x + cur_tile_x_ + 0.5f;
    y = tile_.y + cur_tile_y_ + 0.5f;
  } else {
    // TODO: This currently samples randomly, but ideally we'd also support
    // stratified sampling.
    float u = rand_.Next();
    float v = rand_.Next();
    x = tile_.x + cur_tile_x_ + u;
    y = tile_.y + cur_tile_y_ + v;

    // At times, we'll overshoot onto the next pixel due to rounding error, so
    // we try to detect this and round to negative infinity if it has occurred.
    if (x == (tile_.x + cur_tile_x_ + 1.0f)) {
      x = std::nextafter(x, -std::numeric_limits<float>::infinity());
    }
    if (y == (tile_.y + cur_tile_y_ + 1.0f)) {
      y = std::nextafter(y, -std::numeric_limits<float>::infinity());
    }
  }

  ++cur_pixel_sample_;
  if (cur_pixel_sample_ == pixel_samples_) {
    ++cur_tile_x_;
    cur_pixel_sample_ = 0;
  }
  if (cur_tile_x_ == tile_.width) {
    ++cur_tile_y_;
    cur_tile_x_ = 0;
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
