#include "muon/parser.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "muon/strings.h"
#include "third_party/glm/glm.hpp"

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

void logBadLine(std::string line) {
  LOG(WARNING) << "Malformed input line: " << line;
}

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
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.width = width;
      scene.height = height;
      break;
    }
    case ParseCmd::kMaxDepth: {
      int max_depth;
      iss >> max_depth;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.max_depth = max_depth;
      break;
    }
    case ParseCmd::kOutput: {
      std::string output;
      iss >> output;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.output = output;
      break;
    }
      // Camera commands.
    case ParseCmd::kCamera: {
      float eyex, eyey, eyez, lookatx, lookaty, lookatz, upx, upy, upz, fov;
      iss >> eyex >> eyey >> eyez >> lookatx >> lookaty >> lookatz >> upx >>
          upy >> upz >> fov;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      // TODO: Make sure that scene.width and scene.height have been set.
      scene.camera = absl::make_unique<Camera>(
          glm::vec3(eyex, eyey, eyez), glm::vec3(lookatx, lookaty, lookatz),
          glm::vec3(upx, upy, upz), fov, scene.width, scene.height);
      // TODO: Push transform
      break;
    }
      // Geometry commands.
    case ParseCmd::kSphere: {
      float x, y, z, radius;
      iss >> x >> y >> z >> radius;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      auto sphere = absl::make_unique<Sphere>(glm::vec3(x, y, z), radius);
      scene.AddObject(std::move(sphere));
      break;
    }
    case ParseCmd::kVertex: {
      float x, y, z;
      iss >> x >> y >> z;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      Vertex vert;
      vert.pos = glm::vec3(x, y, z);
      scene.AddVertex(vert);
      break;
    }
    case ParseCmd::kVertexNormal: {
      // TODO
      break;
    }
    case ParseCmd::kTri: {
      int v0, v1, v2;
      iss >> v0 >> v1 >> v2;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      auto tri = absl::make_unique<Tri>(scene.vertices(), v0, v1, v2);
      scene.AddObject(std::move(tri));
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
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.ambient = glm::vec3(r, g, b);
      break;
    }
      // Material commands.
    case ParseCmd::kDiffuse: {
      float r, g, b;
      iss >> r >> g >> b;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.diffuse = glm::vec3(r, g, b);
      break;
    }
    case ParseCmd::kSpecular: {
      float r, g, b;
      iss >> r >> g >> b;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.specular = glm::vec3(r, g, b);
      break;
    }
    case ParseCmd::kShininess: {
      float shininess;
      iss >> shininess;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.shininess = shininess;
      break;
    }
    case ParseCmd::kEmission: {
      float r, g, b;
      iss >> r >> g >> b;
      if (iss.fail()) {
        logBadLine(line);
        break;
      }
      scene.emission = glm::vec3(r, g, b);
      break;
    }
    }
  }

  return scene;
}

} // namespace muon
