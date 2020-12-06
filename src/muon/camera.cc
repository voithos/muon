#include "muon/camera.h"

namespace muon {

Camera::Camera(glm::vec3 eye, glm::vec3 look_at, glm::vec3 up, float fov,
               int width, int height)
    : eye_(eye), look_at_(look_at), up_(up), fov_(fov), width_(width),
      height_(height) {
  // Field of view is given in the y axis, so calculate it for x based on
  // the aspect ratio.
  fov_y_rad_ = glm::radians(fov);
  fov_x_rad_ = fov_y_rad_ * (static_cast<float>(width) / height);

  // Create a coordinate frame.
  glm::vec3 a = eye - look_at;
  glm::vec3 b = up;

  w_ = glm::normalize(a);
  u_ = glm::normalize(glm::cross(b, w_));
  v_ = glm::cross(w_, u_);
}

Ray Camera::CastRay(float x, float y) {
  float half_width = width_ / 2.0f;
  float half_height = height_ / 2.0f;
  float tan_fov_x = glm::tan(fov_x_rad_ / 2.0f);
  float tan_fov_y = glm::tan(fov_y_rad_ / 2.0f);

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

  float alpha = tan_fov_x * offset_x;
  float beta = tan_fov_y * offset_y;

  // We subtract w because the camera is facing in the minus-w direction, by
  // convention.
  glm::vec3 dir = glm::normalize(alpha * u_ + beta * v_ - w_);

  return Ray(eye_, dir);
}

} // namespace muon
