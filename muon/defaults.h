#ifndef MUON_DEFAULTS_H_
#define MUON_DEFAULTS_H_

#include "third_party/glm/glm.hpp"

namespace muon {
namespace defaults {

// Default values for raytracer primitives.

// The scene width, in pixels.
constexpr int kSceneWidth = 150;

// The scene height, in pixels.
constexpr int kSceneHeight = 150;

// The maximum tracing depth.
constexpr int kMaxDepth = 5;

// The output file name.
constexpr char kOutput[] = "raytrace.png";

// Light attenuation, in terms of constant, linear, and quadratic.
static const glm::vec3 kAttenuation = glm::vec3(1.0f, 0.0f, 0.0f);

// Material properties.
static const glm::vec3 kAmbient = glm::vec3(0.2f);
static const glm::vec3 kDiffuse = glm::vec3(0.0f);
static const glm::vec3 kSpecular = glm::vec3(0.0f);
static const glm::vec3 kEmission = glm::vec3(0.0f);

constexpr float kShininess = 1.0f;

} // namespace defaults
} // namespace muon

#endif