#ifndef MUON_EVAL_H_
#define MUON_EVAL_H_

#include "muon/camera.h"
#include "muon/scene.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Evaluates a ray against a given scene.
class Evaluator {
public:
  explicit Evaluator(const Scene &scene) : scene_(scene) {}

  // Evaluates a ray against the scene and returns a traced color.
  glm::vec3 Eval(const Ray &ray);

private:
  const Scene &scene_;
};

} // namespace muon

#endif
