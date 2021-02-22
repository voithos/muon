#ifndef MUON_DEFAULTS_H_
#define MUON_DEFAULTS_H_

#include "muon/brdf_type.h"
#include "muon/importance_sampling.h"
#include "third_party/glm/glm.hpp"

namespace muon {
namespace defaults {

// Default values for raytracer primitives.

// The scene width, in pixels.
constexpr int kSceneWidth = 150;

// The scene height, in pixels.
constexpr int kSceneHeight = 150;

// The minimum tracing depth (inclusive).
constexpr int kMinDepth = 0;

// The maximum tracing depth (inclusive).
constexpr int kMaxDepth = 5;

// The output file name.
constexpr char kOutput[] = "raytrace.png";

// The gamma of the final image.
constexpr float kGamma = 1.0f;

// Whether or not to compute vertex normals.
constexpr bool kComputeVertexNormals = false;

// The number of samples per pixel.
constexpr int kSamplesPerPixel = 1;

// The number of samples per light.
constexpr int kLightSamples = 1;

// Whether or not to stratify light sampling.
constexpr bool kLightStratify = false;

// Whether or not to enable next event estimation.
constexpr bool kNextEventEstimation = false;

// Whether or not to enable Russian Roulette.
constexpr bool kRussianRoulette = false;

// The default importance sampling approach.
constexpr ImportanceSampling kImportanceSampling =
    ImportanceSampling::kHemisphere;

// Light attenuation, in terms of constant, linear, and quadratic.
static const glm::vec3 kAttenuation = glm::vec3(1.0f, 0.0f, 0.0f);

// Material properties.
static const glm::vec3 kAmbient = glm::vec3(0.2f);
static const glm::vec3 kDiffuse = glm::vec3(0.0f);
static const glm::vec3 kSpecular = glm::vec3(0.0f);
static const glm::vec3 kEmission = glm::vec3(0.0f);

// Shininess exponent.
constexpr float kShininess = 1.0f;

// Surface roughness, in the range [0, 1].
constexpr float kRoughness = 0.0f;

// The default BRDF to use.
constexpr BRDFType kBRDF = BRDFType::kPhong;

}  // namespace defaults
}  // namespace muon

#endif
