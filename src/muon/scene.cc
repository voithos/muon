#include "muon/scene.h"

namespace muon {

absl::optional<Intersection> Tri::Intersect(const Ray &ray) {
  return absl::nullopt;
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

  glm::vec3 dir_to_center = ray.origin() - pos;
  float b_prime = glm::dot(ray.direction(), dir_to_center);
  float c = glm::dot(dir_to_center, dir_to_center) - radius * radius;

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

  glm::vec3 intersection_pos = ray.origin() + ray.direction() * t;
  glm::vec3 intersection_normal = glm::normalize(intersection_pos - pos);

  // TODO: Handle transforms
  return Intersection{
      .distance = t,
      .pos = intersection_pos,
      .normal = intersection_normal,
  };
}

void Scene::AddVertex(Vertex vert) { vertices_.push_back(vert); }

void Scene::AddObject(std::unique_ptr<SceneObject> obj) {
  // Apply current cached lighting.
  obj->ambient = ambient;
  obj->diffuse = diffuse;
  obj->specular = specular;
  obj->emission = emission;
  obj->shininess = shininess;
  // obj.transform = ...
  // TODO

  objects_.push_back(std::move(obj));
}

} // namespace muon
