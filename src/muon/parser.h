#ifndef MUON_PARSER_H_
#define MUON_PARSER_H_

#include <string>

#include "muon/scene.h"

namespace muon {

// Parses a scene file into Scene format.
class Parser {
public:
  // Initializes a new Parser with the given scene file.
  explicit Parser(std::string scene_file) : scene_file_(scene_file) {}
  Parser(Parser &&other) = default;
  Parser &operator=(Parser &&other) = default;

  // Parses the scene file and returns corresponding Scene.
  Scene Parse();

private:
  std::string scene_file_;
};

} // namespace muon

#endif
