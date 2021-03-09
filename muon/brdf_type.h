#ifndef MUON_BRDF_TYPE_H_
#define MUON_BRDF_TYPE_H_

namespace muon {

// The types of BRDFs available.
enum class BRDFType {
  // Uses the simple Lambertian BRDF.
  kLambertian = 0,
  // Uses a modified Phong BRDF.
  kPhong,
  // Uses the GGX specular BRDF.
  kGGX,
};

}  // namespace muon

#endif
