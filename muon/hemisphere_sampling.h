#ifndef MUON_HEMISPHERE_SAMPLING_H_
#define MUON_HEMISPHERE_SAMPLING_H_

#include "muon/random.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// Uniformly samples a unit hemisphere centered about the given normal.
glm::vec3 SampleHemisphere(const glm::vec3& normal, UniformRandom& rand);

// Samples the unit hemisphere according to the cosine of the normal.
glm::vec3 SampleCosine(const glm::vec3& normal, UniformRandom& rand);

}  // namespace muon

#endif
