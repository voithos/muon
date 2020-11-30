#ifndef MUON_FILM_H_
#define MUON_FILM_H_

#include <string>

#include "third_party/cimg/CImg.h"

namespace muon {

const int kImageLayers = 1;
const int kNumColors = 3;

// Stores temporary image data and handles persisting said data.
class Film {
public:
  // Initializes a new Film with a given width and height and a path to an
  // output file, which should be a png.
  Film(int width, int height, std::string output_file)
      : width_(width), height_(height), output_file_(output_file),
        output_(width, height, kImageLayers, kNumColors, /* default */ 0){};
  Film(Film &&other) = default;
  Film &operator=(Film &&other) = default;

  // Sets a specific pixel coordinate to a given color.
  void SetPixel(int x, int y, float r, float g, float b);

  // Writes the sampled output to disk.
  void WriteOutput();

private:
  int width_;
  int height_;
  std::string output_file_;
  cimg_library::CImg<unsigned char> output_;
};

} // namespace muon

#endif
