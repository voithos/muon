#ifndef MUON_BRDF_TYPE_H_
#define MUON_BRDF_TYPE_H_

namespace muon {

// The types of BRDFs available.
enum class BRDFType {
  // Uses a modified Phong BRDF.
  kPhong = 0,
  // Uses the GGX specular BRDF.
  kGGX,
};

}  // namespace muon

#endif
