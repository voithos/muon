#ifndef MUON_PARSER_H_
#define MUON_PARSER_H_

#include <memory>
#include <string>

#include "muon/acceleration.h"
#include "muon/acceleration_type.h"
#include "muon/integration.h"
#include "muon/materials.h"
#include "muon/options.h"
#include "muon/scene.h"
#include "muon/stats.h"

namespace muon {

// Represents a working area used while parsing.
class ParsingWorkspace {
 public:
  // Material properties.
  std::shared_ptr<Material> material;

  // The scene.
  std::unique_ptr<Scene> scene;

  // Acceleration structure.
  std::unique_ptr<acceleration::Structure> accel;

  // Configured integrator.
  std::unique_ptr<Integrator> integrator;

  // Multiplies the top of the stack with the given transform matrix.
  void MultiplyTransform(const glm::mat4 &m);
  // Pushes the current transform on to the stack.
  void PushTransform();
  // Pops the current transform from the stack.
  void PopTransform();
  // Copies the current material into a new shared material. This should be
  // called prior to modifications to the material.
  void GenMaterial();

  // Applies current working properties to the given primitive.
  void UpdatePrimitive(Primitive &obj);

 private:
  // Transform stack.
  std::vector<glm::mat4> transforms_ = {glm::mat4(1.0f)};
};

// Represents a configuration of a scene along with its supporting structures.
struct SceneConfig {
  std::unique_ptr<Scene> scene;
  std::unique_ptr<Integrator> integrator;
};

// Parses a scene file into Scene format.
class Parser {
 public:
  // Initializes a new Parser with the given scene file.
  Parser(std::string scene_file, const Options &options, Stats &stats)
      : scene_file_(scene_file), options_(options), stats_(stats) {}

  // Parses the scene file and returns corresponding Scene.
  SceneConfig Parse();

 private:
  std::string scene_file_;
  const Options &options_;
  Stats &stats_;

  void ApplyDefaults(ParsingWorkspace &workspace) const;
  std::unique_ptr<acceleration::Structure> CreateAccelerationStructure() const;
};

}  // namespace muon

#endif
