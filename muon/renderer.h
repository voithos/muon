#ifndef MUON_RENDERER_H_
#define MUON_RENDERER_H_

#include <string>

#include "muon/debug.h"
#include "muon/options.h"

namespace muon {

class Renderer {
 public:
  Renderer(std::string scene_file, const Options& options)
      : scene_file_(scene_file), options_(options) {
    debug::MaybeEnableFloatingPointExceptions();
  }

  // Runs the ray tracer based on the renderer's configuration.
  void Render() const;

 private:
  std::string scene_file_;
  const Options& options_;
};

}  // namespace muon

#endif
