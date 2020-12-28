#include "muon/film.h"

#include <stdexcept>

#include "absl/strings/str_format.h"
#include "glog/logging.h"

namespace muon {

void Film::SetPixel(int x, int y, glm::vec3 color) {
  // Verify bounds of x and y.
  if (x < 0 || y < 0 || x >= width_ || y >= height_) {
    throw std::invalid_argument(
        absl::StrFormat("Invalid call to SetPixel(); (x=%d, y=%d) out of "
                        "bounds of image size (%d, %d)",
                        x, y, width_, height_));
  }
  color = glm::clamp(color, 0.0f, 1.0f);
  output_(x, y, 0) = color.r * 255;
  output_(x, y, 1) = color.g * 255;
  output_(x, y, 2) = color.b * 255;
}

void Film::WriteOutput() {
  VLOG(1) << "Writing to output: " << output_file_;
  output_.save(output_file_.c_str());
}

}  // namespace muon
