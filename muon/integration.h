#ifndef MUON_INTEGRATION_H_
#define MUON_INTEGRATION_H_

#include <random>

#include "muon/camera.h"
#include "muon/scene.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Integrates lighting contributions for a ray against a scene.
class Integrator {
 public:
  explicit Integrator(Scene &scene) : scene_(scene) {}
  virtual ~Integrator() {}

  // Traces a ray against the scene and returns a traced color.
  glm::vec3 Trace(const Ray &ray);
  glm::vec3 Trace(const Ray &ray, const int depth);

 protected:
  // Shades an intersection based on the current scene lighting, returning the
  // resulting color.
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) = 0;

  Scene &scene_;
};

// A simple integrator that approximates the rendering equation by tracing rays
// directly and shading using a simple Phong lighting model.
class Raytracer : public Integrator {
 public:
  explicit Raytracer(Scene &scene) : Integrator(scene) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) override;
};

// An analytic integrator that uses a simplified Lambertian BRDF with area
// light sources to calculate direct lighting contributions. This does not take
// visibility into account, and does not do global illumination.
class AnalyticDirect : public Integrator {
 public:
  explicit AnalyticDirect(Scene &scene) : Integrator(scene) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) override;
};

// Base class for Monte Carlo based integrators.
class MonteCarlo : public Integrator {
 public:
  explicit MonteCarlo(Scene &scene)
      : Integrator(scene), gen_(rd_()), rand_(0.0f, 1.0f) {}

 protected:
  // RNG and seed device for monte carlo. We use the Mersenne Twister because
  // it has high-quality characteristics.
  std::random_device rd_;
  std::mt19937 gen_;
  std::uniform_real_distribution<float> rand_;
};

// A Monte Carlo integrator that calculates direct lighting contributions only.
class MonteCarloDirect : public MonteCarlo {
 public:
  explicit MonteCarloDirect(Scene &scene) : MonteCarlo(scene) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) override;

  // Shades an intersection with only the direct lighting contribution, without
  // any indirect recursion.
  glm::vec3 ShadeDirect(const Intersection &hit, const glm::vec3 &shift_pos,
                        const glm::vec3 &reflected_dir);
};

// A Monte Carlo based path tracer that handles indirect lighting.
class PathTracer : public MonteCarloDirect {
 public:
  explicit PathTracer(Scene &scene) : MonteCarloDirect(scene) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) override;

 private:
  // Uniformly samples a unit hemisphere centered about the given normal.
  glm::vec3 SampleHemisphere(const glm::vec3 &normal);
};

}  // namespace muon

#endif
