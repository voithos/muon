#include "muon/film.h"

namespace muon {
void Film::SetPixel(int x, int y, float r, float g, float b) {
  // TODO: Verify bounds of x+y and rgb.
  output_(x, y, kImageLayers, 0) = r * 255;
  output_(x, y, kImageLayers, 1) = g * 255;
  output_(x, y, kImageLayers, 2) = b * 255;
}

void Film::WriteOutput() { output_.save(output_file_.c_str()); }
} // namespace muon
