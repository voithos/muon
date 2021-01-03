#ifndef MUON_LIGHTING_H_
#define MUON_LIGHTING_H_

#include "muon/camera.h"
#include "muon/objects.h"
#include "muon/types.h"
#include "third_party/glm/glm.hpp"

namespace muon {

// TODO: This should really be in a separate geometry library; possibly need to
// think of a better way to expose this data besides ShadingInfo.
struct Quad {
  // TODO: Corner represents the top-left corner of the quad, edge0 is the left
  // edge, edge1 is the top edge, light comes out the front. Does this make
  // sense...?
  glm::vec3 corner;
  glm::vec3 edge0;
  glm::vec3 edge1;
};

struct ShadingInfo {
  // Color of the light, taking into account attenuation.
  glm::vec3 color;
  // Direction to the light from the intersection.
  glm::vec3 direction;
  // Distance to the light from the intersection.
  float distance;
  // Optional quad that represents the light shape.
  Quad *area;
};

class Light {
 public:
  explicit Light(glm::vec3 color) : color_(color) {}
  virtual ~Light() {}

  // Calculates information for later shading computation based on an
  // intersection point.
  virtual ShadingInfo ShadingInfoAt(const glm::vec3 &pos) = 0;

 protected:
  glm::vec3 color_;
};

class DirectionalLight : public Light {
 public:
  DirectionalLight(glm::vec3 color, glm::vec3 direction)
      : Light(color), direction_(direction) {}

  ShadingInfo ShadingInfoAt(const glm::vec3 &pos) override;

 private:
  glm::vec3 direction_;
};

class PointLight : public Light {
 public:
  PointLight(glm::vec3 color, glm::vec3 pos, glm::vec3 attenuation)
      : Light(color), pos_(pos), attenuation_(attenuation) {}

  ShadingInfo ShadingInfoAt(const glm::vec3 &pos) override;

 private:
  glm::vec3 pos_;
  glm::vec3 attenuation_;
};

// A parallelogram area light represented by a corner vertex and two edge
// vectors.
class QuadLight : public Light {
 public:
  QuadLight(glm::vec3 color, glm::vec3 corner, glm::vec3 edge0, glm::vec3 edge1)
      : Light(color), area_{.corner = corner, .edge0 = edge0, .edge1 = edge1} {}

  ShadingInfo ShadingInfoAt(const glm::vec3 &pos) override;

 private:
  Quad area_;
};

}  // namespace muon

#endif
