#include "muon/camera.h"

#include "muon/transform.h"

namespace muon {

Ray Ray::Transform(const glm::mat4 &transform) const {
  glm::vec3 t_origin = TransformPosition(transform, origin_);
  glm::vec3 t_direction = TransformDirection(transform, direction_);
  return Ray(t_origin, t_direction);
}

glm::vec3 Ray::At(float t) const { return origin_ + direction_ * t; }

Camera::Camera(glm::vec3 eye, glm::vec3 look_at, glm::vec3 up, float fov,
               int width, int height)
    : eye_(eye),
      look_at_(look_at),
      up_(up),
      fov_(fov),
      width_(width),
      height_(height) {
  // Field of view is given in the y axis, so calculate it for x based on
  // the aspect ratio.
  tan_fov_y_ = glm::tan(glm::radians(fov_) / 2.0f);
  tan_fov_x_ = tan_fov_y_ * (static_cast<float>(width_) / height_);

  // Create a coordinate frame.
  glm::vec3 a = eye_ - look_at_;
  glm::vec3 b = up_;

  w_ = glm::normalize(a);
  u_ = glm::normalize(glm::cross(b, w_));
  v_ = glm::cross(w_, u_);
}

Ray Camera::CastRay(float x, float y) const {
  float half_width = width_ / 2.0f;
  float half_height = height_ / 2.0f;

  // Calculate components along the orthogonal directions (that is, u and v in
  // the coordinate frame). In terms of the viewing plane, we calculate alpha
  // corresponding to the horizontal axis (width), and beta corresponding to
  // the vertical axis (height). Both are based on an offset from the center,
  // in the range [-1, 1], which we then multiply with the maximum offset based
  // on the FOV.

  float offset_x = (x - half_width) / half_width;
  // The y offset is reversed since we typically want y=0 to represent the top
  // of the viewing plane, not the bottom, which is the opposite of the
  // direction of the axis.
  float offset_y = (half_height - y) / half_height;

  float alpha = tan_fov_x_ * offset_x;
  float beta = tan_fov_y_ * offset_y;

  // We subtract w because the camera is facing in the minus-w direction, by
  // convention.
  glm::vec3 dir = glm::normalize(alpha * u_ + beta * v_ - w_);

  return Ray(eye_, dir);
}

}  // namespace muon
