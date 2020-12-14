#ifndef MUON_RENDERER_H_
#define MUON_RENDERER_H_

#include <string>

namespace muon {

class Renderer {
public:
  Renderer(std::string scene_file, bool show_stats)
      : scene_file_(scene_file), show_stats_(show_stats) {}

  // Runs the ray tracer based on the renderer's configuration.
  void Render();

private:
  std::string scene_file_;
  bool show_stats_;
};

} // namespace muon

#endif
