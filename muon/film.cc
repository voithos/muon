#include "muon/film.h"

#include <stdexcept>

#include "absl/strings/str_format.h"
#include "glog/logging.h"

namespace muon {

void Film::SetPixel(size_t x, size_t y, glm::vec3 color) {
  // Verify bounds of x and y.
  if (x < 0 || y < 0 || x >= width_ || y >= height_) {
    throw std::invalid_argument(
        absl::StrFormat("Invalid call to SetPixel(); (x=%d, y=%d) out of "
                        "bounds of image size (%d, %d)",
                        x, y, width_, height_));
  }
  accumulator_[x][y] += color;
}

void Film::WriteOutput() {
  VLOG(1) << "Writing to output: " << output_file_;

  for (size_t x = 0; x < width_; ++x) {
    for (size_t y = 0; y < height_; ++y) {
      glm::vec3 value =
          accumulator_[x][y] / static_cast<float>(pixel_samples_);

      // Gamma correction.
      // TODO: Pull this out into a post-process system.
      value.r = glm::pow(value.r, 1.0f / gamma_);
      value.g = glm::pow(value.g, 1.0f / gamma_);
      value.b = glm::pow(value.b, 1.0f / gamma_);

      glm::vec3 color = glm::clamp(value, 0.0f, 1.0f);
      output_(x, y, 0) = color.r * 255;
      output_(x, y, 1) = color.g * 255;
      output_(x, y, 2) = color.b * 255;
    }
  }

  output_.save(output_file_.c_str());
}

}  // namespace muon
