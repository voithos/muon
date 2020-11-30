#include "muon/parser.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "glog/logging.h"
#include "muon/strings.h"
#include "third_party/glm/vec3.hpp"

namespace muon {

enum class ParseCmd {
  kIgnored = 0, // Ignored.
  // General commands.
  kSize,
  kMaxDepth,
  kOutput,
  // Camera commands.
  kCamera,
  // Geometry commands.
  kSphere,
  kVertex,
  kVertexNormal,
  kTri,
  kTriNormal,
  // Transformation commands.
  kTranslate,
  kRotate,
  kScale,
  kPushTransform,
  kPopTransform,
  // Light commands.
  kDirectional,
  kPoint,
  kAttenuation,
  kAmbient,
  // Material commands.
  kDiffuse,
  kSpecular,
  kShininess,
  kEmission,
};

std::map<std::string, ParseCmd> command_map = {
    {"size", ParseCmd::kSize},
    {"maxdepth", ParseCmd::kMaxDepth},
    {"output", ParseCmd::kOutput},
    {"camera", ParseCmd::kCamera},
    {"sphere", ParseCmd::kSphere},
    {"maxverts", ParseCmd::kIgnored},
    {"maxvertnorms", ParseCmd::kIgnored},
    {"vertex", ParseCmd::kVertex},
    {"vertexnormal", ParseCmd::kVertexNormal},
    {"tri", ParseCmd::kTri},
    {"trinormal", ParseCmd::kTriNormal},
    {"translate", ParseCmd::kTranslate},
    {"rotate", ParseCmd::kRotate},
    {"scale", ParseCmd::kScale},
    {"pushTransform", ParseCmd::kPushTransform},
    {"popTransform", ParseCmd::kPopTransform},
    {"directional", ParseCmd::kDirectional},
    {"point", ParseCmd::kPoint},
    {"attenuation", ParseCmd::kAttenuation},
    {"ambient", ParseCmd::kAmbient},
    {"diffuse", ParseCmd::kDiffuse},
    {"specular", ParseCmd::kSpecular},
    {"shininess", ParseCmd::kShininess},
    {"emission", ParseCmd::kEmission},
};

Scene Parser::Parse() {
  Scene scene;

  VLOG(1) << "Reading from input: " << scene_file_;

  std::ifstream infile(scene_file_);
  std::string line;
  while (std::getline(infile, line)) {
    trim(line);

    // Ignore empty lines and comments.
    if (line.size() == 0 || line[0] == '#') {
      continue;
    }

    VLOG(2) << "Read line: " << line;
    std::istringstream iss(line);

    // Extract command.
    std::string cmd;
    iss >> cmd;

    auto it = command_map.find(cmd);
    if (it == command_map.end()) {
      LOG(WARNING) << "Unknown command: " << cmd;
      continue;
    }

    switch (it->second) {
    case ParseCmd::kIgnored: {
      // Ignored.
      break;
    }
      // General commands.
    case ParseCmd::kSize: {
      int width, height;
      iss >> width >> height;
      scene.width = width;
      scene.height = height;
      break;
    }
    case ParseCmd::kMaxDepth: {
      int max_depth;
      iss >> max_depth;
      scene.max_depth = max_depth;
      break;
    }
    case ParseCmd::kOutput: {
      std::string output;
      iss >> output;
      scene.output = output;
      break;
    }
      // Camera commands.
    case ParseCmd::kCamera: {
      // TODO
      break;
    }
      // Geometry commands.
    case ParseCmd::kSphere: {
      // TODO
      break;
    }
    case ParseCmd::kVertex: {
      // TODO
      break;
    }
    case ParseCmd::kVertexNormal: {
      // TODO
      break;
    }
    case ParseCmd::kTri: {
      // TODO
      break;
    }
    case ParseCmd::kTriNormal: {
      // TODO
      break;
    }
      // Transformation commands.
    case ParseCmd::kTranslate: {
      // TODO
      break;
    }
    case ParseCmd::kRotate: {
      // TODO
      break;
    }
    case ParseCmd::kScale: {
      // TODO
      break;
    }
    case ParseCmd::kPushTransform: {
      // TODO
      break;
    }
    case ParseCmd::kPopTransform: {
      // TODO
      break;
    }
      // Light commands.
    case ParseCmd::kDirectional: {
      // TODO
      break;
    }
    case ParseCmd::kPoint: {
      // TODO
      break;
    }
    case ParseCmd::kAttenuation: {
      // TODO
      break;
    }
    case ParseCmd::kAmbient: {
      float r, g, b;
      iss >> r >> g >> b;
      scene.ambient = glm::vec3(r, g, b);
      break;
    }
      // Material commands.
    case ParseCmd::kDiffuse: {
      float r, g, b;
      iss >> r >> g >> b;
      scene.diffuse = glm::vec3(r, g, b);
      break;
    }
    case ParseCmd::kSpecular: {
      float r, g, b;
      iss >> r >> g >> b;
      scene.specular = glm::vec3(r, g, b);
      break;
    }
    case ParseCmd::kShininess: {
      float shininess;
      iss >> shininess;
      scene.shininess = shininess;
      break;
    }
    case ParseCmd::kEmission: {
      float r, g, b;
      iss >> r >> g >> b;
      scene.emission = glm::vec3(r, g, b);
      break;
    }
    }
  }

  return scene;
}
} // namespace muon
