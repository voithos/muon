#include "muon/parser.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "absl/memory/memory.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "glog/logging.h"
#include "muon/acceleration.h"
#include "muon/brdf_type.h"
#include "muon/defaults.h"
#include "muon/lighting.h"
#include "muon/objects.h"
#include "muon/random.h"
#include "muon/strings.h"
#include "third_party/glm/glm.hpp"
#include "third_party/glm/gtx/norm.hpp"
#include "third_party/glm/gtx/transform.hpp"

namespace muon {

enum class ParseCmd {
  kIgnored = 0,  // Ignored.
  // General commands.
  kRandomSeed,
  kFilmSize,
  kMinDepth,
  kMaxDepth,
  kOutput,
  kGamma,
  // Integrator commands.
  kIntegrator,
  kPixelSamples,
  kLightSamples,
  kLightStratify,
  kNextEventEstimation,
  kRussianRoulette,
  kImportanceSampling,
  // Camera commands.
  kCamera,
  // External commands.
  kLoad,
  // Geometry commands.
  kComputeVertexNormals,
  kSphere,
  kStartMesh,
  kEndMesh,
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
  kDirectionalLight,
  kPointLight,
  kAttenuation,
  kQuadLight,
  // Material commands.
  kBRDF,
  kAmbient,
  kDiffuse,
  kSpecular,
  kShininess,
  kRoughness,
  kEmission,
};

std::map<std::string, ParseCmd> command_map = {
    {"random_seed", ParseCmd::kRandomSeed},
    {"film_size", ParseCmd::kFilmSize},
    {"min_depth", ParseCmd::kMinDepth},
    {"max_depth", ParseCmd::kMaxDepth},
    {"output", ParseCmd::kOutput},
    {"gamma", ParseCmd::kGamma},
    {"integrator", ParseCmd::kIntegrator},
    {"pixel_samples", ParseCmd::kPixelSamples},
    {"light_samples", ParseCmd::kLightSamples},
    {"light_stratify", ParseCmd::kLightStratify},
    {"next_event_estimation", ParseCmd::kNextEventEstimation},
    {"russian_roulette", ParseCmd::kRussianRoulette},
    {"importance_sampling", ParseCmd::kImportanceSampling},
    {"camera", ParseCmd::kCamera},
    {"load", ParseCmd::kLoad},
    {"compute_vertex_normals", ParseCmd::kComputeVertexNormals},
    {"sphere", ParseCmd::kSphere},
    {"start_mesh", ParseCmd::kStartMesh},
    {"end_mesh", ParseCmd::kEndMesh},
    {"vertex", ParseCmd::kVertex},
    {"vertex_normal", ParseCmd::kVertexNormal},
    {"tri", ParseCmd::kTri},
    {"tri_normal", ParseCmd::kTriNormal},
    {"translate", ParseCmd::kTranslate},
    {"rotate", ParseCmd::kRotate},
    {"scale", ParseCmd::kScale},
    {"push_transform", ParseCmd::kPushTransform},
    {"pop_transform", ParseCmd::kPopTransform},
    {"directional_light", ParseCmd::kDirectionalLight},
    {"point_light", ParseCmd::kPointLight},
    {"attenuation", ParseCmd::kAttenuation},
    {"quad_light", ParseCmd::kQuadLight},
    {"brdf", ParseCmd::kBRDF},
    {"ambient", ParseCmd::kAmbient},
    {"diffuse", ParseCmd::kDiffuse},
    {"specular", ParseCmd::kSpecular},
    {"shininess", ParseCmd::kShininess},
    {"roughness", ParseCmd::kRoughness},
    {"emission", ParseCmd::kEmission},
};

void ParsingWorkspace::MultiplyTransform(const glm::mat4 &m) {
  transforms_.back() = std::make_shared<glm::mat4>(*transforms_.back() * m);
  UpdateCachedTransforms();
  VLOG(3) << "  Current transform: \n" << pprint(*transforms_.back());
}

void ParsingWorkspace::PushTransform() {
  // TODO: Add checks for these transform methods.
  transforms_.push_back(transforms_.back());
  UpdateCachedTransforms();
  VLOG(3) << "  Transform stack size: " << transforms_.size();
  VLOG(3) << "  Current transform: \n" << pprint(*transforms_.back());
}

void ParsingWorkspace::PopTransform() {
  transforms_.pop_back();
  UpdateCachedTransforms();
  VLOG(3) << "  Transform stack size: " << transforms_.size();
}

void ParsingWorkspace::GenMaterial() {
  auto m = std::make_shared<Material>(*material);
  material = m;
}

void ParsingWorkspace::UpdatePrimitive(Primitive &obj) {
  obj.material = material;

  obj.transform = transforms_.back();
  obj.inv_transform = inv_transform_;
  obj.inv_transpose_transform = inv_transpose_transform_;
}

void ParsingWorkspace::UpdateCachedTransforms() {
  inv_transform_ =
      std::make_shared<glm::mat4>(glm::inverse(*transforms_.back()));
  inv_transpose_transform_ =
      std::make_shared<glm::mat4>(glm::transpose(*inv_transform_));
}

void logBadLine(std::string line) {
  LOG(WARNING) << "Malformed input line: " << line;
}

std::unique_ptr<brdf::BRDF> CreateBRDF(BRDFType type) {
  std::unique_ptr<brdf::BRDF> brdf;
  switch (type) {
    case BRDFType::kLambertian:
      brdf = absl::make_unique<brdf::Lambertian>();
    case BRDFType::kPhong:
      brdf = absl::make_unique<brdf::Phong>();
      break;
    case BRDFType::kGGX:
      brdf = absl::make_unique<brdf::GGX>();
      break;
  }
  return brdf;
}

void Parser::ApplyDefaults(ParsingWorkspace &ws) const {
  ws.seedgen =
      absl::make_unique<SeedGenerator>(SeedGenerator::GenerateTrueRandomSeed());

  ws.material = std::make_shared<Material>();
  ws.material->ambient = defaults::kAmbient;
  ws.material->diffuse = defaults::kDiffuse;
  ws.material->specular = defaults::kSpecular;
  ws.material->emission = defaults::kEmission;
  ws.material->shininess = defaults::kShininess;
  ws.material->roughness = defaults::kRoughness;
  ws.material->SetBRDF(CreateBRDF(defaults::kBRDF));

  ws.scene = absl::make_unique<Scene>();
  ws.accel = CreateAccelerationStructure();
  ws.integrator = absl::make_unique<Raytracer>(*ws.scene);

  ws.scene->width = defaults::kSceneWidth;
  ws.scene->height = defaults::kSceneHeight;
  ws.scene->min_depth = defaults::kMinDepth;
  ws.scene->max_depth = defaults::kMaxDepth;
  ws.scene->output = defaults::kOutput;
  ws.scene->gamma = defaults::kGamma;
  ws.scene->compute_vertex_normals = defaults::kComputeVertexNormals;
  ws.scene->pixel_samples = defaults::kPixelSamples;
  ws.scene->light_samples = defaults::kLightSamples;
  ws.scene->light_stratify = defaults::kLightStratify;
  ws.scene->next_event_estimation = defaults::kNextEventEstimation;
  ws.scene->russian_roulette = defaults::kRussianRoulette;
  ws.scene->importance_sampling = defaults::kImportanceSampling;
  ws.scene->attenuation = defaults::kAttenuation;
}

std::unique_ptr<acceleration::Structure> Parser::CreateAccelerationStructure()
    const {
  std::unique_ptr<acceleration::Structure> accel;
  switch (options_.acceleration) {
    case AccelerationType::kLinear:
      accel = absl::make_unique<acceleration::Linear>();
      break;
    case AccelerationType::kBVH:
      accel = absl::make_unique<acceleration::BVH>(options_.partition_strategy);
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
      case ParseCmd::kRandomSeed: {
        unsigned int seed;
        iss >> seed;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.seedgen = absl::make_unique<SeedGenerator>(seed);
      }
      case ParseCmd::kFilmSize: {
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
      case ParseCmd::kMinDepth: {
        int min_depth;
        iss >> min_depth;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.scene->min_depth = min_depth;
        break;
      }
      case ParseCmd::kMaxDepth: {
        // TODO: Currently enabling Russian Roulette requires setting maxdepth
        // to -1 to work properly; make this more automatic.
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
      case ParseCmd::kGamma: {
        float gamma;
        iss >> gamma;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.scene->gamma = gamma;
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
        if (type == "normals") {
          ws.integrator = absl::make_unique<NormalsTracer>(*ws.scene);
        } else if (type == "albedo") {
          ws.integrator = absl::make_unique<AlbedoTracer>(*ws.scene);
        } else if (type == "depth") {
          ws.integrator = absl::make_unique<DepthTracer>(*ws.scene);
        } else if (type == "raytracer") {
          ws.integrator = absl::make_unique<Raytracer>(*ws.scene);
        } else if (type == "analyticdirect") {
          ws.integrator = absl::make_unique<AnalyticDirect>(*ws.scene);
        } else if (type == "pathtracer") {
          ws.integrator =
              absl::make_unique<PathTracer>(*ws.scene, ws.seedgen->Next());
        } else {
          logBadLine(line);
          break;
        }
        break;
      }
      case ParseCmd::kPixelSamples: {
        int pixel_samples;
        iss >> pixel_samples;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.scene->pixel_samples = pixel_samples;
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
      case ParseCmd::kNextEventEstimation: {
        std::string next_event_estimation;
        iss >> next_event_estimation;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (next_event_estimation == "off") {
          ws.scene->next_event_estimation = NEE::kOff;
        } else if (next_event_estimation == "on") {
          ws.scene->next_event_estimation = NEE::kOn;
        } else if (next_event_estimation == "mis") {
          ws.scene->next_event_estimation = NEE::kMIS;
        } else {
          logBadLine(line);
          break;
        }
        break;
      }
      case ParseCmd::kRussianRoulette: {
        std::string russian_roulette;
        iss >> russian_roulette;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (russian_roulette == "on") {
          ws.scene->russian_roulette = true;
        } else if (russian_roulette == "off") {
          ws.scene->russian_roulette = false;
        } else {
          logBadLine(line);
          break;
        }
        break;
      }
      case ParseCmd::kImportanceSampling: {
        std::string importance_sampling;
        iss >> importance_sampling;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (importance_sampling == "hemisphere") {
          ws.scene->importance_sampling = ImportanceSampling::kHemisphere;
        } else if (importance_sampling == "cosine") {
          ws.scene->importance_sampling = ImportanceSampling::kCosine;
        } else if (importance_sampling == "brdf") {
          ws.scene->importance_sampling = ImportanceSampling::kBRDF;
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
        // External commands.
      case ParseCmd::kLoad: {
        std::string filename;
        iss >> filename;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        // Attempt to load file.
        // TODO: Move this to a separate importer library.
        VLOG(3) << "Loading external file: " << filename;
        std::filesystem::path p = scene_file_;
        Assimp::Importer importer;
        const aiScene *scene =
            importer.ReadFile(p.parent_path() / filename,
                              aiProcessPreset_TargetRealtime_MaxQuality);
        if (!scene) {
          LOG(WARNING) << "Error during load: " << importer.GetErrorString();
          break;
        }
        // TODO: Instead of just loading meshes without a transform, load the
        // assimp scene's hierarchical nodes.
        if (scene->mRootNode != nullptr) {
          VLOG(3) << "Root node contains " << scene->mRootNode->mNumChildren
                  << " children";
          auto &trans = scene->mRootNode->mTransformation;
          // clang-format off
          VLOG(3) << "Root node transform: \n"
            << trans.a1 << " " << trans.a2 << " " << trans.a3 << " " << trans.a4 << "\n"
            << trans.b1 << " " << trans.b2 << " " << trans.b3 << " " << trans.b4 << "\n"
            << trans.c1 << " " << trans.c2 << " " << trans.c3 << " " << trans.c4 << "\n"
            << trans.d1 << " " << trans.d2 << " " << trans.d3 << " " << trans.d4 << "\n";
          // clang-format on
        }
        if (scene->HasMeshes()) {
          VLOG(3) << "  Contains " << scene->mNumMeshes << " meshes";
          for (unsigned int mesh_idx = 0; mesh_idx < scene->mNumMeshes;
               ++mesh_idx) {
            const aiMesh *mesh = scene->mMeshes[mesh_idx];
            if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
              LOG(WARNING)
                  << " Skipping mesh #" << mesh_idx
                  << " (name: " << mesh->mName.C_Str()
                  << "), which contains non-triangular primitive types: "
                  << mesh->mPrimitiveTypes;
              continue;
            }
            if (!mesh->HasFaces()) {
              VLOG(3) << " Skipping mesh #" << mesh_idx << " ("
                      << mesh->mName.C_Str() << "), which contains no faces";
              continue;
            }
            VLOG(3) << "  Mesh #" << mesh_idx << " with " << mesh->mNumVertices
                    << " vertices and " << mesh->mNumFaces << " faces";

            // Create the vertices.
            // TODO: Switch this to use the actual mesh logic.
            std::vector<std::reference_wrapper<Vertex>> vertex_refs;
            const aiVector3D *vertices = mesh->mVertices;
            for (unsigned int vertex_idx = 0; vertex_idx < mesh->mNumVertices;
                 ++vertex_idx) {
              Vertex &v = ws.scene->GenVertex();
              v.pos.x = vertices[vertex_idx].x;
              v.pos.y = vertices[vertex_idx].y;
              v.pos.z = vertices[vertex_idx].z;
              vertex_refs.push_back(v);
            }

            // Create the tris.
            for (unsigned int tri_idx = 0; tri_idx < mesh->mNumFaces;
                 ++tri_idx) {
              const aiFace &face = mesh->mFaces[tri_idx];
              if (face.mNumIndices != 3) {
                LOG(WARNING) << "  Encountered a non-triangle face!";
                break;
              }
              const unsigned int v0 = face.mIndices[0];
              const unsigned int v1 = face.mIndices[1];
              const unsigned int v2 = face.mIndices[2];

              auto tri = absl::make_unique<Tri>(
                  vertex_refs[v0].get(), vertex_refs[v1].get(),
                  vertex_refs[v2].get(), ws.scene->compute_vertex_normals);
              ws.UpdatePrimitive(*tri);
              if (ws.scene->compute_vertex_normals) {
                // Compute vertex normals additively. We will later need to
                // normalize these.
                auto normal = tri->Normal();
                vertex_refs[v0].get().normal += normal;
                vertex_refs[v1].get().normal += normal;
                vertex_refs[v2].get().normal += normal;
              }
              ws.accel->AddPrimitive(std::move(tri));
            }
          }
        }
        break;
      }
        // Geometry commands.
      case ParseCmd::kComputeVertexNormals: {
        std::string compute_vertex_normals;
        iss >> compute_vertex_normals;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (compute_vertex_normals == "on") {
          ws.scene->compute_vertex_normals = true;
        } else if (compute_vertex_normals == "off") {
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
      case ParseCmd::kStartMesh: {
        ws.scene->StartMesh();
        break;
      }
      case ParseCmd::kEndMesh: {
        ws.scene->EndMesh();
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
      case ParseCmd::kDirectionalLight: {
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
      case ParseCmd::kPointLight: {
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

        // Also create two tris to represent the area light itself.
        Vertex &va = ws.scene->GenVertex();
        Vertex &vb = ws.scene->GenVertex();
        Vertex &vc = ws.scene->GenVertex();
        Vertex &vd = ws.scene->GenVertex();
        va.pos = corner;
        vb.pos = corner + edge0;
        vc.pos = corner + edge1;
        vd.pos = corner + edge0 + edge1;

        // TODO: This is pretty gross. Can we improve this?
        auto tri0 = absl::make_unique<Tri>(va, vc, vb, false);
        auto tri1 = absl::make_unique<Tri>(vb, vc, vd, false);
        auto material = std::make_shared<Material>();
        material->SetBRDF(CreateBRDF(defaults::kBRDF));
        material->emission = color;  // Emit based on color.
        std::shared_ptr<glm::mat4> identity = std::make_shared<glm::mat4>(1.0f);
        tri0->material = material;
        tri0->light = light.get();
        tri0->transform = identity;
        tri0->inv_transform = identity;
        tri0->inv_transpose_transform = identity;
        tri1->material = material;
        tri1->light = light.get();
        tri1->transform = identity;
        tri1->inv_transform = identity;
        tri1->inv_transpose_transform = identity;
        ws.accel->AddPrimitive(std::move(tri0));
        ws.accel->AddPrimitive(std::move(tri1));

        ws.scene->AddLight(std::move(light));
        break;
      }
        // Material commands.
      case ParseCmd::kBRDF: {
        BRDFType type;
        std::string brdf;
        iss >> brdf;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        if (brdf == "lambertian") {
          type = BRDFType::kLambertian;
        } else if (brdf == "phong") {
          type = BRDFType::kPhong;
        } else if (brdf == "ggx") {
          type = BRDFType::kGGX;
        } else {
          logBadLine(line);
          break;
        }
        ws.GenMaterial();
        ws.material->SetBRDF(CreateBRDF(type));
        break;
      }
      case ParseCmd::kAmbient: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.GenMaterial();
        ws.material->ambient = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kDiffuse: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.GenMaterial();
        ws.material->diffuse = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kSpecular: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.GenMaterial();
        ws.material->specular = glm::vec3(r, g, b);
        break;
      }
      case ParseCmd::kShininess: {
        float shininess;
        iss >> shininess;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.GenMaterial();
        ws.material->shininess = shininess;
        break;
      }
      case ParseCmd::kRoughness: {
        float roughness;
        iss >> roughness;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.GenMaterial();
        ws.material->roughness = roughness;
        break;
      }
      case ParseCmd::kEmission: {
        float r, g, b;
        iss >> r >> g >> b;
        if (iss.fail()) {
          logBadLine(line);
          break;
        }
        ws.GenMaterial();
        ws.material->emission = glm::vec3(r, g, b);
        break;
      }
    }
  }

  // TODO: Maybe we should do this unconditionally, since we need normals to be
  // unit length anyway?
  if (ws.scene->compute_vertex_normals) {
    // Normalize vertex normals for all meshes.
    for (auto &mesh : ws.scene->meshes()) {
      for (auto &vertex : mesh) {
        if (glm::length2(vertex.normal) > 0.0f) {
          vertex.normal = glm::normalize(vertex.normal);
        }
      }
    }
  }

  ws.accel->Init();
  ws.scene->seedgen = std::move(ws.seedgen);
  ws.scene->root = std::move(ws.accel);
  return {
      .scene = std::move(ws.scene),
      .integrator_prototype = std::move(ws.integrator),
  };
}

}  // namespace muon
