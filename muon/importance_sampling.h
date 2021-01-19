#ifndef MUON_IMPORTANCE_SAMPLING_H_
#define MUON_IMPORTANCE_SAMPLING_H_

namespace muon {

// The types of importance sampling to use.
enum class ImportanceSampling {
  // Uniform sampling across the hemisphere.
  kHemisphere = 0,
  // Cosine sampling, which works well for near-Lambertian surfaces.
  kCosine,
};

}  // namespace muon

#endif
