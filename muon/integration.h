#ifndef MUON_INTEGRATION_H_
#define MUON_INTEGRATION_H_

#include "muon/camera.h"
#include "muon/scene.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Integrates lighting contributions for a ray against a scene.
class Integrator {
 public:
  explicit Integrator(const Scene &scene) : scene_(scene) {}
  virtual ~Integrator() {}

  // Traces a ray against the scene and returns a traced color.
  glm::vec3 Trace(const Ray &ray) const;
  glm::vec3 Trace(const Ray &ray, const int depth) const;

 protected:
  // Shades an intersection based on the current scene lighting, returning the
  // resulting color.
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) const = 0;

  const Scene &scene_;
};

// A simple integrator that approximates the rendering equation by tracing rays
// directly and shading using a simple Phong lighting model.
class Raytracer : public Integrator {
 public:
  explicit Raytracer(const Scene &scene) : Integrator(scene) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) const override;
};

// An analytic integrator that uses a simplified Lambertian BRDF with area
// light sources to calculate direct lighting contributions. This does not take
// visibility into account, and does not do global illumination.
class AnalyticDirect : public Integrator {
 public:
  explicit AnalyticDirect(const Scene &scene) : Integrator(scene) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) const override;
};

}  // namespace muon

#endif
