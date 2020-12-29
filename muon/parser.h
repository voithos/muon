#ifndef MUON_PARSER_H_
#define MUON_PARSER_H_

#include <string>

#include "muon/acceleration.h"
#include "muon/acceleration_type.h"
#include "muon/options.h"
#include "muon/scene.h"
#include "muon/stats.h"

namespace muon {

namespace {
// Represents a working area used while parsing.
class ParsingWorkspace {
 public:
  // Material properties.
  Material material;

  // Acceleration structure.
  std::unique_ptr<acceleration::Structure> accel;

  // Multiplies the top of the stack with the given transform matrix.
  void MultiplyTransform(const glm::mat4 &m);
  // Pushes the current transform on to the stack.
  void PushTransform();
  // Pops the current transform from the stack.
  void PopTransform();

  // Applies current working properties to the given primitive.
  void UpdatePrimitive(Primitive &obj);

 private:
  // Transform stack.
  std::vector<glm::mat4> transforms_ = {glm::mat4(1.0f)};
};
};  // namespace

// Parses a scene file into Scene format.
class Parser {
 public:
  // Initializes a new Parser with the given scene file.
  Parser(std::string scene_file, const Options &options, Stats &stats)
      : scene_file_(scene_file), options_(options), stats_(stats) {}

  // Parses the scene file and returns corresponding Scene.
  Scene Parse();

 private:
  std::string scene_file_;
  const Options &options_;
  Stats &stats_;

  void ApplyDefaults(ParsingWorkspace &workspace, Scene &scene) const;
  std::unique_ptr<acceleration::Structure> CreateAccelerationStructure() const;
};

}  // namespace muon

#endif
