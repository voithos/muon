#include "muon/scene.h"

#include "glog/logging.h"
#include "muon/strings.h"

namespace muon {

constexpr float kEpsilon = 0.0000001;

Tri::Tri(const std::vector<Vertex> &vertices, int v0, int v1, int v2)
    : vertices_(vertices), v0_(v0), v1_(v1), v2_(v2) {
  // Calculate the surface normal by computing the cross product of the
  // triangle's edges.
  const glm::vec3 &edge_ba = vertices_[v1_].pos - vertices_[v0_].pos;
  const glm::vec3 &edge_ca = vertices_[v2_].pos - vertices_[v0_].pos;
  normal_ = glm::cross(edge_ba, edge_ca);
  normal_length2_ = glm::dot(normal_, normal_);
}

absl::optional<Intersection> Tri::Intersect(const Ray &ray) {
  // A tri can be conceptualized as a bounded plane. A plane can be represented
  // as:
  //   (P - A) • n = 0
  // where P is a point on the plane, A is one of the vertices, and n is the
  // normal of the plane. This is also equivalently written as:
  //   P • n = A • n
  //
  // Given a ray `P = P_0 + P_1*t`, where P_0 is the origin and P_1 is the
  // direction, we can substitute it into the plane equation in place of the
  // point P:
  //   (P_0 + P_1*t) • n = A • n
  //
  // Solving for t:
  //   t = (A • n - P_0 • n) / (P_1 • n)
  // If `P_1 • n` is 0, it means that the ray is parallel to the plane, so
  // there is no intersection. Otherwise, there is.
  //
  // Next, we must determine if the ray actually hit inside the triangle.
  // We can find barycentric coordinates for the intersection point:
  //   P = u * A + v * B + w * C
  // where u, v, and w are all nonnegative and sum up to 1, and [A, B, C] are
  // the vertices.
  //
  // If we treat the triangle as a 2d coordinate system with origin at A, we
  // can define any point inside the triangle as a sum of components AB and AC,
  // which we can call u and v. In practice, we don't need w explicitly, and so
  // our coordinates can become:
  //   P = (1 - u - v) * A + u * B + v * C
  //
  // We can determine u and v by calculating the ratio between the area of the
  // triangle created by two vertices + our intersection point, as compared to
  // the area of the tri as a whole. In practice, we don't need to compute the
  // actual areas, but instead can compare the magnitudes of the cross products
  // of the relevant edges.
  //
  // We don't need to compute w explicitly, but we do need to check that it
  // actually lies inside the triangle, which we can do for an arbitrary
  // triangle edge by computing the cross product of the edge with an analogous
  // edge formed with the point P. We then compute the dot product between this
  // new vector and our triangle's normal -- if the result is negative, then it
  // means that the point was outside (or, to the "right" of the triangle's
  // edge, based on the right-hand-rule). Incidentally, we can reuse this
  // computation to then find u and v.

  // Inverse transform the ray to make the intersection test simpler.
  Ray t_ray = ray.Transform(inv_transform);

  // First, we find an intersection with the triangle's plane.
  float dir_along_normal = glm::dot(normal_, t_ray.direction());
  if (dir_along_normal > -kEpsilon && dir_along_normal < kEpsilon) {
    // The ray is parallel, so they don't intersect.
    return absl::nullopt;
  }

  const glm::vec3 &a = vertices_[v0_].pos;
  const glm::vec3 &b = vertices_[v1_].pos;
  const glm::vec3 &c = vertices_[v2_].pos;

  float t = (glm::dot(a, normal_) - glm::dot(t_ray.origin(), normal_)) /
            dir_along_normal;
  if (t < 0) {
    // The tri is behind the ray.
    return absl::nullopt;
  }

  glm::vec3 p = t_ray.At(t);

  // Next, for each edge formed by the triangle A B C, we check that the point
  // lies "inside" the triangle. Simultaneously, we compute u, v, and w.
  // See
  // https://www.scratchapixel.com/images/upload/ray-triangle/barycentric.png
  // for a visual representation of this.
  glm::vec3 tri_edge, point_edge, sub_tri_normal;

  // Edge AB.
  tri_edge = b - a;
  point_edge = p - a;
  sub_tri_normal = glm::cross(tri_edge, point_edge);
  float v = glm::dot(normal_, sub_tri_normal);
  if (v < 0) {
    // P is on the outside of this edge.
    return absl::nullopt;
  }

  // Edge BC.
  tri_edge = c - b;
  point_edge = p - b;
  sub_tri_normal = glm::cross(tri_edge, point_edge);
  float w = glm::dot(normal_, sub_tri_normal);
  if (w < 0) {
    // P is on the outside of this edge.
    return absl::nullopt;
  }

  // Edge CA.
  tri_edge = a - c;
  point_edge = p - c;
  sub_tri_normal = glm::cross(tri_edge, point_edge);
  float u = glm::dot(normal_, sub_tri_normal);
  if (u < 0) {
    // P is on the outside of this edge.
    return absl::nullopt;
  }

  // TODO: Use these.
  u /= normal_length2_;
  v /= normal_length2_;
  w /= normal_length2_;

  // Bring the intersection point and normal back to a transformed state.
  glm::vec3 world_p = transform * glm::vec4(p, 1.0f);
  glm::vec3 world_n = glm::normalize(inv_transpose_transform *
                                     glm::vec4(glm::normalize(normal_), 0.0f));
  // Compute the world distance now that we have the world intersection point.
  float world_t = glm::length(world_p - ray.origin());

  return Intersection{
      .distance = world_t,
      .pos = world_p,
      .normal = world_n,
  };
}

absl::optional<Intersection> Sphere::Intersect(const Ray &ray) {
  // A sphere can be conceptualized as:
  //   (P - C) • (P - C) = r^2
  // where P is a point on the sphere, C is the center of the sphere, and r is
  // the radius.
  //
  // Given a ray `P = P_0 + P_1*t`, where P_0 is the origin and P_1 is the
  // direction, we can substitute it into the sphere equation in place of the
  // point P:
  //   (P_0 + P_1*t - C) • (P_0 + P_1*t - C) - r^2 = 0
  //
  // The only unknown is the distance along the ray, t. Expanding it, we can
  // see that it is a quadratic equation of the standard form `ax^2 + bx + c =
  // 0`, where x = t:
  //   (P_1 • P_1) * t^2 + 2 * (P_1 • (P_0 - C)) * t + (P_0 - C) • (P_0 - C) -
  //   r^2 = 0
  //
  // Now we need to solve for t using the standard formula:
  //   t = (-b +- sqrt(b^2 - 4ac)) / 2a
  //
  // The `a` component involves a dot product between two identical unit
  // vectors, which is 1, so we can ignore it:
  //   t = (-b +- sqrt(b^2 - 4c)) / 2
  //
  // Expanding it out, we notice that we can factor out a coefficient of 2 from
  // both `b` and the sqrt, which then cancels with the divisor, finally
  // leaving us with:
  //   t = -b' +- sqrt(b'^2 - c)
  // where
  //   b' = (P_1 • (P_0 - C)), and
  //   c = (P_0 - C) • (P_0 - C) - r^2
  //
  // Expanding it all out, we can now solve for t:
  //   t = -(P_1 • (P_0 - C)) +- sqrt((P_1 • (P_0 - C))^2 - ((P_0 - C) • (P_0 -
  //   C) - r^2))

  // Inverse transform the ray to make the intersection test simpler.
  Ray t_ray = ray.Transform(inv_transform);

  glm::vec3 dir_to_center = t_ray.origin() - pos_;
  float b_prime = glm::dot(t_ray.direction(), dir_to_center);
  float c = glm::dot(dir_to_center, dir_to_center) - radius_ * radius_;

  // First check the discriminant.
  float discriminant = b_prime * b_prime - c;
  if (discriminant < 0) {
    // No hits.
    return absl::nullopt;
  }

  // At this point, we know there is an intersection!
  // Calculate the roots.
  float sqrt_d = glm::sqrt(discriminant);
  float root_0 = -b_prime + sqrt_d;
  float root_1 = -b_prime - sqrt_d;

  // If both roots are negative, then the sphere is behind the ray.
  if (root_0 < 0 && root_1 < 0) {
    return absl::nullopt;
  }

  // Otherwise, we pick the smallest (i.e. closest) _positive_ root,
  // corresponding to the surface we hit. If one of the roots is negative, then
  // it means that the ray started inside the sphere.
  float t = glm::min(root_0, root_1);
  if (t < 0) {
    t = glm::max(root_0, root_1);
  }

  glm::vec3 p = t_ray.At(t);
  glm::vec3 n = glm::normalize(p - pos_);

  // Bring the intersection point and normal back to a transformed state.
  glm::vec3 world_p = transform * glm::vec4(p, 1.0f);
  glm::vec3 world_n =
      glm::normalize(inv_transpose_transform * glm::vec4(n, 0.0f));
  // Compute the world distance now that we have the world intersection point.
  float world_t = glm::length(world_p - ray.origin());

  return Intersection{
      .distance = world_t,
      .pos = world_p,
      .normal = world_n,
  };
}

void Scene::AddVertex(Vertex vert) { vertices_.push_back(vert); }

void Scene::AddObject(std::shared_ptr<SceneObject> obj) {
  // Apply current cached lighting.
  obj->ambient = ambient;
  obj->diffuse = diffuse;
  obj->specular = specular;
  obj->emission = emission;
  obj->shininess = shininess;

  obj->transform = transforms_.back();
  obj->inv_transform = glm::inverse(obj->transform);
  obj->inv_transpose_transform = glm::transpose(obj->inv_transform);

  objects_.push_back(obj);
}

void Scene::AddLight(std::shared_ptr<Light> light) { lights_.push_back(light); }

void Scene::MultiplyTransform(const glm::mat4 &m) {
  transforms_.back() = transforms_.back() * m;
  VLOG(3) << "  Current transform: \n" << pprint(transforms_.back());
}

void Scene::PushTransform() {
  transforms_.push_back(transforms_.back());
  VLOG(3) << "  Transform stack size: " << transforms_.size();
  VLOG(3) << "  Current transform: \n" << pprint(transforms_.back());
}

void Scene::PopTransform() {
  transforms_.pop_back();
  VLOG(3) << "  Transform stack size: " << transforms_.size();
}

} // namespace muon
