#ifndef MUON_RENDERER_H_
#define MUON_RENDERER_H_

#include <string>

#include "muon/options.h"

namespace muon {

class Renderer {
 public:
  Renderer(std::string scene_file, const Options& options)
      : scene_file_(scene_file), options_(options) {}

  // Runs the ray tracer based on the renderer's configuration.
  void Render();

 private:
  std::string scene_file_;
  const Options& options_;
};

}  // namespace muon

#endif
