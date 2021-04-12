#ifndef MUON_NEE_H_
#define MUON_NEE_H_

namespace muon {

// The possible configuration of next event estimation (NEE).
enum class NEE {
  // Next event estimation is disabled.
  kOff = 0,
  // Next event estimation is enabled.
  kOn,
  // Next event estimation is BRDF importance sampled via MIS.
  kMIS,
};

}  // namespace muon

#endif
