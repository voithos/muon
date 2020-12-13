#ifndef MUON_CAMERA_H_
#define MUON_CAMERA_H_

#include "third_party/glm/glm.hpp"

namespace muon {

class Ray {
public:
  Ray(const glm::vec3 &origin, const glm::vec3 &direction)
      : origin_(origin), direction_(direction) {}
  glm::vec3 origin() const { return origin_; }
  glm::vec3 direction() const { return direction_; }

  // Transforms the ray using the given transformation matrix, returning a new
  // ray.
  Ray Transform(const glm::mat4 &transform) const;

  // Returns a point on the ray at a given "distance" t.
  glm::vec3 At(float t) const;

private:
  glm::vec3 origin_;
  glm::vec3 direction_;
};

class Camera {
public:
  Camera(glm::vec3 eye, glm::vec3 look_at, glm::vec3 up, float fov, int width,
         int height);

  // Generates a Ray through the viewport at (x, y) in screen space.
  Ray CastRay(float x, float y);

private:
  glm::vec3 eye_;
  glm::vec3 look_at_;
  glm::vec3 up_;
  float fov_; // Degrees, in the y axis.
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

} // namespace muon

#endif
