#ifndef MUON_INTEGRATION_H_
#define MUON_INTEGRATION_H_

#include <memory>
#include <random>

#include "muon/acceleration.h"
#include "muon/camera.h"
#include "muon/random.h"
#include "muon/scene.h"
#include "muon/stats.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Integrates lighting contributions for a ray against a scene.
class Integrator {
 public:
  Integrator(const Integrator &other) : scene_(other.scene_) {}

  explicit Integrator(Scene &scene)
      : scene_(scene),
        // The scene may not be fully formed at the time of construction, so we
        // save any important work to Init().
        workspace_(nullptr) {}

  virtual ~Integrator() = default;

  // Initializes the integrator. Should be called for each unique instance.
  virtual void Init() { workspace_ = scene_.root->CreateWorkspace(); }

  // Traces a ray against the scene and returns a traced color.
  glm::vec3 Trace(const Ray &ray);
  glm::vec3 Trace(const Ray &ray, const glm::vec3 &throughput, const int depth);

  TraceStats trace_stats() { return workspace_->stats; }

  // Clones the integrator.
  virtual std::unique_ptr<Integrator> Clone() const = 0;

 protected:
  // Shades an intersection based on the current scene lighting, returning the
  // resulting color.
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput, const int depth) = 0;

  Scene &scene_;
  std::unique_ptr<acceleration::Workspace> workspace_;
};

// A debug integrator that renders the normals of the scene.
class NormalsTracer : public Integrator {
 public:
  NormalsTracer(const NormalsTracer &other) : Integrator(other) {}
  explicit NormalsTracer(Scene &scene) : Integrator(scene) {}
  virtual std::unique_ptr<Integrator> Clone() const override;

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;
};

// A debug integrator that renders the depth of the scene.
class DepthTracer : public Integrator {
 public:
  DepthTracer(const DepthTracer &other) : Integrator(other) {}
  explicit DepthTracer(Scene &scene) : Integrator(scene) {}
  virtual std::unique_ptr<Integrator> Clone() const override;

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
  Raytracer(const Raytracer &other) : Integrator(other) {}
  explicit Raytracer(Scene &scene) : Integrator(scene) {}
  virtual std::unique_ptr<Integrator> Clone() const override;

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
  AnalyticDirect(const AnalyticDirect &other) : Integrator(other) {}
  explicit AnalyticDirect(Scene &scene) : Integrator(scene) {}
  virtual std::unique_ptr<Integrator> Clone() const override;

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;
};

// A Monte Carlo based path tracer that handles global illumination.
class PathTracer : public Integrator {
 public:
  PathTracer(const PathTracer &other) : Integrator(other) {}
  explicit PathTracer(Scene &scene) : Integrator(scene) {}
  virtual std::unique_ptr<Integrator> Clone() const override;

 protected:
  virtual glm::vec3 Shade(const Intersection &hit, const Ray &ray,
                          const glm::vec3 &throughput,
                          const int depth) override;

 private:
  // Shades an intersection with only the indirect light contribution.
  glm::vec3 ShadeIndirect(const Intersection &hit, const glm::vec3 &shift_pos,
                          const Ray &ray, const glm::vec3 &throughput,
                          const int depth);

  // Shades an intersection with only direct light contribution.
  glm::vec3 ShadeDirect(const Intersection &hit, const glm::vec3 &shift_pos,
                        const Ray &ray, const glm::vec3 &throughput);

  // Shades an intersection with only the direct lighting contribution via Next
  // Event Estimation, without any indirect recursion.
  glm::vec3 ShadeDirectNEE(const Intersection &hit, const glm::vec3 &shift_pos,
                           const Ray &ray, const glm::vec3 &throughput);

  // Shades an intersection with only the direct lighting contribution, modeled
  // via BRDF multiple importance sampling.
  glm::vec3 ShadeDirectBRDFMIS(const Intersection &hit,
                               const glm::vec3 &shift_pos, const Ray &ray,
                               const glm::vec3 &throughput);

  // Shades an intersection with only the direct lighting contribution, modeled
  // via NEE multiple importance sampling.
  glm::vec3 ShadeDirectNEEMIS(const Intersection &hit,
                              const glm::vec3 &shift_pos, const Ray &ray,
                              const glm::vec3 &throughput);

  // Samples a reflected ray, outputting its direction, pdf, and computed BRDF
  // throughput (taking into account current throughput). Returns a boolean
  // indicating if the sample is valid (e.g. above the horizon).
  bool SampleReflection(const Intersection &hit, const Ray &ray,
                        const glm::vec3 &throughput, glm::vec3 &sampled_dir,
                        float &pdf, glm::vec3 &next_throughput);

  // Computes the combined PDF of all lights for a given sample direction.
  // TODO: This should be part of the lighting system instead.
  float NEEPDF(const Ray &sampled_ray, const glm::vec3 &hit_pos);

  // Computes the importance sampling PDF of a sample based on scene config.
  // TODO: Make BRDFs const.
  float ImportanceSamplingPDF(const glm::vec3 &sampled_dir,
                              const Intersection &hit, const Ray &ray,
                              brdf::BRDF &brdf);

  // RNG for monte carlo.
  UniformRandom rand_;
};

}  // namespace muon

#endif
