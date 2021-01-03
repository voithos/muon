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
  // Integrator commands.
  kIntegrator,
  kLightSamples,
  kLightStratify,
  // Camera commands.
  kCamera,
  // Geometry commands.
  kComputeVertexNormals,
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
  kQuadLight,
  // Material commands.
  kAmbient,
  kDiffuse,
  kSpecular,
  kShininess,
  kEmission,
};

std::map<std::string, ParseCmd> command_map = {
    {"size", ParseCmd::kSize},
    {"maxdepth", ParseCmd::kMaxDepth},
    {"output", ParseCmd::kOutput},
    {"integrator", ParseCmd::kIntegrator},
    {"lightsamples", ParseCmd::kLightSamples},
    {"lightstratify", ParseCmd::kLightStratify},
    {"camera", ParseCmd::kCamera},
    {"computeVertexNormals", ParseCmd::kComputeVertexNormals},
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
    {"quadLight", ParseCmd::kQuadLight},
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

void Parser::ApplyDefaults(ParsingWorkspace &ws) const {
  ws.material.ambient = defaults::kAmbient;
  ws.material.diffuse = defaults::kDiffuse;
  ws.material.specular = defaults::kSpecular;
  ws.material.emission = defaults::kEmission;
  ws.material.shininess = defaults::kShininess;

  ws.scene = absl::make_unique<Scene>();
  ws.accel = CreateAccelerationStructure();
  ws.integrator = absl::make_unique<Raytracer>(*ws.scene);

  ws.scene->width = defaults::kSceneWidth;
  ws.scene->height = defaults::kSceneHeight;
  ws.scene->max_depth = defaults::kMaxDepth;
  ws.scene->output = defaults::kOutput;
  ws.scene->compute_vertex_normals = defaults::kComputeVertexNormals;
  ws.scene->light_samples = defaults::kLightSamples;
  ws.scene->light_stratify = defaults::kLightStratify;
  ws.scene->attenuation = defaults::kAttenuation;
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

// TODO: Instead of constructing the scene in-line, we should pull this into an
// intermediate format and build the scene from that. That way future supported
// file types don't need duplicate construction logic (only parsing logic).
SceneConfig Parser::Parse() {
  // Keep track of a temporary workspace in addition to the scene that we're
  // building.
  ParsingWorkspace ws;
  ApplyDefaults(ws);

  VLOG(1) << "Reading from input: " << scene_file_;

  std::ifstream infile(scene_file_);
  std::string line;
  while (std::getline(infile, line)) {
    trim(line);

    // Ignore empty lines and comments.
    if (line.size() == 0 || line[0] == '#') {
      continue;
    }

    VLOG(3) << "Read line: " << line;
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
        ws.scene->width = width;
        ws.scene->height = height;
        break;
      }
      case ParseCmd::kMaxDepth: {
        int max_depth;
        iss >> max_depth;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.scene->max_depth = max_depth;
        break;
      }
      case ParseCmd::kOutput: {
        std::string output;
        iss >> output;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.scene->output = output;
        break;
      }
        // Integrator commands.
      case ParseCmd::kIntegrator: {
        std::string type;
        iss >> type;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (type == "raytracer") {
          ws.integrator = absl::make_unique<Raytracer>(*ws.scene);
        } else if (type == "analyticdirect") {
          ws.integrator = absl::make_unique<AnalyticDirect>(*ws.scene);
        } else if (type == "direct") {
          ws.integrator = absl::make_unique<MonteCarloDirect>(*ws.scene);
        } else {
          logBadLine(line);
          break;
        }
        break;
      }
      case ParseCmd::kLightSamples: {
        int light_samples;
        iss >> light_samples;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.scene->light_samples = light_samples;
        break;
      }
      case ParseCmd::kLightStratify: {
        std::string light_stratify;
        iss >> light_stratify;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (light_stratify == "on") {
          ws.scene->light_stratify = true;
        } else if (light_stratify == "off") {
          ws.scene->light_stratify = false;
        } else {
          logBadLine(line);
          break;
        }
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
        ws.scene->camera = absl::make_unique<Camera>(
            glm::vec3(eyex, eyey, eyez), glm::vec3(lookatx, lookaty, lookatz),
            glm::vec3(upx, upy, upz), fov, ws.scene->width, ws.scene->height);
        // Preserve the identity matrix.
        ws.PushTransform();
        break;
      }
        // Geometry commands.
      case ParseCmd::kComputeVertexNormals: {
        std::string vertex_normals;
        iss >> vertex_normals;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (vertex_normals == "on") {
          ws.scene->compute_vertex_normals = true;
        } else if (vertex_normals == "off") {
          ws.scene->compute_vertex_normals = false;
        } else {
          logBadLine(line);
          break;
        }
        break;
      }
      case ParseCmd::kSphere: {
        float x, y, z, radius;
        iss >> x >> y >> z >> radius;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        auto sphere = absl::make_unique<Sphere>(glm::vec3(x, y, z), radius);
        ws.UpdatePrimitive(*sphere);
        ws.accel->AddPrimitive(std::move(sphere));
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
        // We initialize the normals later.
        vert.normal = glm::vec3(0.0f);
        ws.scene->AddVertex(vert);
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
        auto &vertices = ws.scene->vertices();
        auto tri =
            absl::make_unique<Tri>(vertices[v0], vertices[v1], vertices[v2],
                                   ws.scene->compute_vertex_normals);
        ws.UpdatePrimitive(*tri);
        if (ws.scene->compute_vertex_normals) {
          // Compute vertex normals additively. We will later need to normalize
          // these.
          auto normal = tri->Normal();
          vertices[v0].normal += normal;
          vertices[v1].normal += normal;
          vertices[v2].normal += normal;
        }
        ws.accel->AddPrimitive(std::move(tri));
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
        ws.MultiplyTransform(glm::translate(glm::vec3(x, y, z)));
        break;
      }
      case ParseCmd::kRotate: {
        float x, y, z, angle;
        iss >> x >> y >> z >> angle;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.MultiplyTransform(
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
        ws.MultiplyTransform(glm::scale(glm::vec3(x, y, z)));
        break;
      }
      case ParseCmd::kPushTransform: {
        ws.PushTransform();
        break;
      }
      case ParseCmd::kPopTransform: {
        ws.PopTransform();
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
        ws.scene->AddLight(std::move(light));
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
            glm::vec3(r, g, b), glm::vec3(x, y, z), ws.scene->attenuation);
        ws.scene->AddLight(std::move(light));
        break;
      }
      case ParseCmd::kAttenuation: {
        float constant, linear, quadratic;
        iss >> constant >> linear >> quadratic;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.scene->attenuation = glm::vec3(constant, linear, quadratic);
        break;
      }
      case ParseCmd::kQuadLight: {
        float corner_x, corner_y, corner_z, edge0_x, edge0_y, edge0_z, edge1_x,
            edge1_y, edge1_z, r, g, b;
        iss >> corner_x >> corner_y >> corner_z >> edge0_x >> edge0_y >>
            edge0_z >> edge1_x >> edge1_y >> edge1_z >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        auto color = glm::vec3(r, g, b);
        auto corner = glm::vec3(corner_x, corner_y, corner_z);
        auto edge0 = glm::vec3(edge0_x, edge0_y, edge0_z);
        auto edge1 = glm::vec3(edge1_x, edge1_y, edge1_z);
        auto light = absl::make_unique<QuadLight>(color, corner, edge0, edge1);
        ws.scene->AddLight(std::move(light));

        // Also create two tris to represent the area light itself.
        Vertex va, vb, vc, vd;
        va.pos = corner;
        vb.pos = corner + edge0;
        vc.pos = corner + edge1;
        vd.pos = corner + edge0 + edge1;

        // TODO: This is pretty gross. Can we improve this?
        auto tri0 = absl::make_unique<Tri>(va, vb, vc, false);
        auto tri1 = absl::make_unique<Tri>(vb, vd, vc, false);
        Material material;
        material.emission = color;  // Emit based on color.
        glm::mat4 identity(1.0f);
        tri0->material = material;
        tri0->transform = identity;
        tri0->inv_transform = identity;
        tri0->inv_transpose_transform = identity;
        tri1->material = material;
        tri1->transform = identity;
        tri1->inv_transform = identity;
        tri1->inv_transpose_transform = identity;
        ws.accel->AddPrimitive(std::move(tri0));
        ws.accel->AddPrimitive(std::move(tri1));
        break;
      }
        // Material commands.
      case ParseCmd::kAmbient: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.material.ambient = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kDiffuse: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.material.diffuse = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kSpecular: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.material.specular = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kShininess: {
        float shininess;
        iss >> shininess;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.material.shininess = shininess;
        break;
      }
      case ParseCmd::kEmission: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.material.emission = glm::vec3(r, g, b);
        break;
      }
    }
  }

  if (ws.scene->compute_vertex_normals) {
    // Normalize vertex normals.
    for (auto &vertex : ws.scene->vertices()) {
      vertex.normal = glm::normalize(vertex.normal);
    }
  }

  ws.accel->Init();
  ws.scene->root = std::move(ws.accel);
  return {
      .scene = std::move(ws.scene),
      .integrator = std::move(ws.integrator),
  };
}

}  // namespace muon
