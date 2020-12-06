#include "muon/film.h"

#include "glog/logging.h"

namespace muon {

void Film::SetPixel(int x, int y, glm::vec3 color) {
  // TODO: Verify bounds of x+y and rgb.
  output_(x, y, 0) = color.r * 255;
  output_(x, y, 1) = color.g * 255;
  output_(x, y, 2) = color.b * 255;
}

void Film::WriteOutput() {
  VLOG(1) << "Writing to output: " << output_file_;
  output_.save(output_file_.c_str());
}

} // namespace muon
