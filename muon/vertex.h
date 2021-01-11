#ifndef MUON_VERTEX_H_
#define MUON_VERTEX_H_

#include "third_party/glm/glm.hpp"

namespace muon {

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
};

}  // namespace muon

#endif
