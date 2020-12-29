#include "muon/parser.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "absl/memory/memory.h"
#include "glog/logging.h"
#include "muon/acceleration.h"
#include "muon/defaults.h"
#include "muon/lighting.h"
#include "muon/objects.h"
#include "muon/strings.h"
#include "third_party/glm/glm.hpp"
#include "third_party/glm/gtx/transform.hpp"

namespace muon {

enum class ParseCmd {
  kIgnored = 0,  // Ignored.
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

void ParsingWorkspace::MultiplyTransform(const glm::mat4 &m) {
  transforms_.back() = transforms_.back() * m;
  VLOG(3) << "  Current transform: \n" << pprint(transforms_.back());
}

void ParsingWorkspace::PushTransform() {
  transforms_.push_back(transforms_.back());
  VLOG(3) << "  Transform stack size: " << transforms_.size();
  VLOG(3) << "  Current transform: \n" << pprint(transforms_.back());
}

void ParsingWorkspace::PopTransform() {
  transforms_.pop_back();
  VLOG(3) << "  Transform stack size: " << transforms_.size();
}

void ParsingWorkspace::UpdatePrimitive(Primitive &obj) {
  obj.material = material;

  obj.transform = transforms_.back();
  obj.inv_transform = glm::inverse(obj.transform);
  obj.inv_transpose_transform = glm::transpose(obj.inv_transform);
}

void logBadLine(std::string line) {
  LOG(WARNING) << "Malformed input line: " << line;
}

void Parser::ApplyDefaults(ParsingWorkspace &workspace, Scene &scene) const {
  workspace.material.ambient = defaults::kAmbient;
  workspace.material.diffuse = defaults::kDiffuse;
  workspace.material.specular = defaults::kSpecular;
  workspace.material.emission = defaults::kEmission;
  workspace.material.shininess = defaults::kShininess;

  workspace.accel = CreateAccelerationStructure();

  scene.width = defaults::kSceneWidth;
  scene.height = defaults::kSceneHeight;
  scene.max_depth = defaults::kMaxDepth;
  scene.output = defaults::kOutput;
  scene.attenuation = defaults::kAttenuation;
}

std::unique_ptr<acceleration::Structure> Parser::CreateAccelerationStructure()
    const {
  std::unique_ptr<acceleration::Structure> accel;
  switch (options_.acceleration) {
    case AccelerationType::kLinear:
      accel = absl::make_unique<acceleration::Linear>(stats_);
      break;
    case AccelerationType::kBVH:
      accel = absl::make_unique<acceleration::BVH>(options_.partition_strategy,
                                                   stats_);
      break;
  }
  return accel;
}

Scene Parser::Parse() {
  // Keep track of a temporary workspace in addition to the scene that we're
  // building.
  ParsingWorkspace workspace;
  Scene scene;
  ApplyDefaults(workspace, scene);

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
        scene.camera = absl::make_unique<Camera>(
            glm::vec3(eyex, eyey, eyez), glm::vec3(lookatx, lookaty, lookatz),
            glm::vec3(upx, upy, upz), fov, scene.width, scene.height);
        // Preserve the identity matrix.
        workspace.PushTransform();
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
        workspace.UpdatePrimitive(*sphere);
        workspace.accel->AddPrimitive(std::move(sphere));
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
        workspace.UpdatePrimitive(*tri);
        workspace.accel->AddPrimitive(std::move(tri));
        break;
      }
      case ParseCmd::kTriNormal: {
        // TODO
        break;
      }
        // Transformation commands.
      case ParseCmd::kTranslate: {
        float x, y, z;
        iss >> x >> y >> z;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        workspace.MultiplyTransform(glm::translate(glm::vec3(x, y, z)));
        break;
      }
      case ParseCmd::kRotate: {
        float x, y, z, angle;
        iss >> x >> y >> z >> angle;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        workspace.MultiplyTransform(
            glm::rotate(glm::radians(angle), glm::vec3(x, y, z)));
        break;
      }
      case ParseCmd::kScale: {
        float x, y, z;
        iss >> x >> y >> z;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        workspace.MultiplyTransform(glm::scale(glm::vec3(x, y, z)));
        break;
      }
      case ParseCmd::kPushTransform: {
        workspace.PushTransform();
        break;
      }
      case ParseCmd::kPopTransform: {
        workspace.PopTransform();
        break;
      }
        // Light commands.
      case ParseCmd::kDirectional: {
        float x, y, z, r, g, b;
        iss >> x >> y >> z >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        auto light = absl::make_unique<DirectionalLight>(glm::vec3(r, g, b),
                                                         glm::vec3(x, y, z));
        scene.AddLight(std::move(light));
        break;
      }
      case ParseCmd::kPoint: {
        float x, y, z, r, g, b;
        iss >> x >> y >> z >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        auto light = absl::make_unique<PointLight>(
            glm::vec3(r, g, b), glm::vec3(x, y, z), scene.attenuation);
        scene.AddLight(std::move(light));
        break;
      }
      case ParseCmd::kAttenuation: {
        float constant, linear, quadratic;
        iss >> constant >> linear >> quadratic;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        scene.attenuation = glm::vec3(constant, linear, quadratic);
        break;
      }
      case ParseCmd::kAmbient: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        workspace.material.ambient = glm::vec3(r, g, b);
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
        workspace.material.diffuse = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kSpecular: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        workspace.material.specular = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kShininess: {
        float shininess;
        iss >> shininess;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        workspace.material.shininess = shininess;
        break;
      }
      case ParseCmd::kEmission: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        workspace.material.emission = glm::vec3(r, g, b);
        break;
      }
    }
  }

  workspace.accel->Init();
  scene.root = std::move(workspace.accel);
  return scene;
}

}  // namespace muon
