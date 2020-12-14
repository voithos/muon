#ifndef MUON_TRACER_H_
#define MUON_TRACER_H_

#include "muon/camera.h"
#include "muon/scene.h"
#include "muon/stats.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Traces a ray against a given scene.
class Tracer {
public:
  explicit Tracer(const Scene &scene, Stats &stats)
      : scene_(scene), stats_(stats) {}

  // Traces a ray against the scene and returns a traced color.
  glm::vec3 Trace(const Ray &ray) const;
  glm::vec3 Trace(const Ray &ray, const int depth) const;

private:
  // Shades an intersection based on the current scene lighting, returning the
  // resulting color.
  glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                  const int depth) const;

  // Checks if a ray to a given target (i.e. distance) is occluded.
  // Used for shadow checks.
  bool IsOccluded(const Ray &shadow_ray, const float light_distance) const;

  const Scene &scene_;
  Stats &stats_;
};

} // namespace muon

#endif
