#include "muon/tracer.h"

#include <iostream>

#include "muon/parser.h"

namespace muon {

void trace(std::string scene_file) {
  Parser p(scene_file);
  Scene d = p.Parse();
}

} // namespace muon
