#include "muon/hemisphere_sampling.h"

#include "muon/transform.h"
#include "third_party/glm/gtc/constants.hpp"

namespace muon {

glm::vec3 SampleHemisphere(const glm::vec3& normal, UniformRandom& rand) {
  // Generate spherical coordinates using two random numbers in [0, 1).
  float r1 = rand.Next();
  float r2 = rand.Next();
  float theta = glm::acos(r1);
  float phi = 2.0f * glm::pi<float>() * r2;

  glm::vec3 s(glm::cos(phi) * glm::sin(theta), glm::sin(phi) * glm::sin(theta),
              glm::cos(theta));

  // We now have a hemisphere sample, but it's centered about the z-axis. We
  // instead want to sample around the hemisphere centered about the normal of
  // the surface, so we want to rotate the sample.
  return RotateToOrthonormalFrame(s, normal);
}

glm::vec3 SampleCosine(const glm::vec3& normal, UniformRandom& rand) {
  // Generate spherical coordinates using two random numbers in [0, 1).
  float r1 = rand.Next();
  float r2 = rand.Next();
  float theta = glm::acos(glm::sqrt(r1));
  float phi = 2.0f * glm::pi<float>() * r2;

  glm::vec3 s(glm::cos(phi) * glm::sin(theta), glm::sin(phi) * glm::sin(theta),
              glm::cos(theta));

  // We now have a hemisphere sample, but it's centered about the z-axis. We
  // instead want to sample around the hemisphere centered about the normal of
  // the surface, so we want to rotate the sample.
  return RotateToOrthonormalFrame(s, normal);
}

}  // namespace muon
