#ifndef MUON_CAMERA_H_
#define MUON_CAMERA_H_

#include "muon/ray.h"
#include "third_party/glm/glm.hpp"

namespace muon {

class Camera {
 public:
  Camera(glm::vec3 eye, glm::vec3 look_at, glm::vec3 up, float fov, int width,
         int height);

  // Generates a Ray through the viewport at (x, y) in screen space.
  Ray CastRay(float x, float y) const;

 private:
  glm::vec3 eye_;
  glm::vec3 look_at_;
  glm::vec3 up_;
  float fov_;  // Degrees, in the y axis.
  // Tangents of the FOV, computed ahead of time.
  float tan_fov_x_;
  float tan_fov_y_;
  int width_;
  int height_;

  // Coordinate frame.
  glm::vec3 u_;
  glm::vec3 v_;
  glm::vec3 w_;
};

}  // namespace muon

#endif
