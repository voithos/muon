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

// A Monte Carlo integrator that calculates direct lighting contributions only.
class MonteCarloDirect : public Integrator {
 public:
  explicit MonteCarloDirect(Scene &scene)
      : Integrator(scene), gen_(rd()), rand_(0.0f, 1.0f) {}

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const int depth) override;
  // RNG and seed device for monte carlo. We use the Mersenne Twister because
  // it has high-quality characteristics.
  std::random_device rd;
  std::mt19937 gen_;
  std::uniform_real_distribution<float> rand_;
};

}  // namespace muon

#endif
