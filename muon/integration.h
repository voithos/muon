#ifndef MUON_INTEGRATION_H_
#define MUON_INTEGRATION_H_

#include <random>

#include "muon/camera.h"
#include "muon/random.h"
#include "muon/scene.h"
#include "muon/stats.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Integrates lighting contributions for a ray against a scene.
class Integrator {
 public:
  Integrator(Scene &scene, Stats &stats) : scene_(scene), stats_(stats) {}
  virtual ~Integrator() {}

  // Traces a ray against the scene and returns a traced color.
  glm::vec3 Trace(const Ray &ray);
  glm::vec3 Trace(const Ray &ray, const glm::vec3 &throughput, const int depth);

 protected:
  // Shades an intersection based on the current scene lighting, returning the
  // resulting color.
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput, const int depth) = 0;

  Scene &scene_;
  Stats &stats_;
};

// A debug integrator that renders the normals of the scene.
class NormalsTracer : public Integrator {
 public:
  NormalsTracer(Scene &scene, Stats &stats) : Integrator(scene, stats) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;
};

// A simple, non-physically-based integrator that approximates the rendering
// equation by tracing rays directly and shading using a simple Phong lighting
// model.
class Raytracer : public Integrator {
 public:
  Raytracer(Scene &scene, Stats &stats) : Integrator(scene, stats) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;
};

// An analytic integrator that uses a simplified Lambertian BRDF with area
// light sources to calculate direct lighting contributions. This does not take
// visibility into account, and does not do global illumination.
class AnalyticDirect : public Integrator {
 public:
  AnalyticDirect(Scene &scene, Stats &stats) : Integrator(scene, stats) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;
};

// Base class for Monte Carlo based integrators.
class MonteCarlo : public Integrator {
 public:
  MonteCarlo(Scene &scene, Stats &stats) : Integrator(scene, stats) {}

 protected:
  // RNG for monte carlo.
  UniformRandom rand_;
};

// A Monte Carlo integrator that calculates direct lighting contributions only.
class MonteCarloDirect : public MonteCarlo {
 public:
  MonteCarloDirect(Scene &scene, Stats &stats) : MonteCarlo(scene, stats) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;

  // Shades an intersection with only the direct lighting contribution, without
  // any indirect recursion.
  glm::vec3 ShadeDirect(const Intersection &hit, const glm::vec3 &shift_pos,
                        const glm::vec3 &reflected_dir,
                        const glm::vec3 &throughput);
};

// A Monte Carlo based path tracer that handles indirect lighting.
class PathTracer : public MonteCarloDirect {
 public:
  PathTracer(Scene &scene, Stats &stats) : MonteCarloDirect(scene, stats) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;

 private:
  // Uniformly samples a unit hemisphere centered about the given normal.
  glm::vec3 SampleHemisphere(const glm::vec3 &normal);
};

}  // namespace muon

#endif
