#ifndef MUON_RENDERER_H_
#define MUON_RENDERER_H_

#include <string>

#include "muon/acceleration_type.h"

namespace muon {

class Renderer {
public:
  Renderer(std::string scene_file, AccelerationType acceleration,
           bool show_stats)
      : scene_file_(scene_file), acceleration_(acceleration),
        show_stats_(show_stats) {}

  // Runs the ray tracer based on the renderer's configuration.
  void Render();

private:
  std::string scene_file_;
  AccelerationType acceleration_;
  bool show_stats_;
};

} // namespace muon

#endif
