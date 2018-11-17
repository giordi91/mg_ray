#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "middleware/glm/glm.hpp"
#include "middleware/json/jsonForwadDeclare.hpp"

namespace mg_ray {
namespace core {

enum class IMPLICIT_MESH_TYPE { SPHERE, PLANE, INVALID };
enum class SHAPE_TYPE { IMPLICIT, POLYGONS, INVALID };
enum class MATERIAL_TYPE { DIFFUSE, METAL, DIALECTRIC, INVALID };

struct SceneCamera {
  glm::mat4x4 view;
  float vFov;
  float aperture;
  float focusDistance;
  float padding;
};

struct SceneMaterial {
  glm::vec3 albedo;
  float glossiness;
  MATERIAL_TYPE type;
};

struct ImplicitSceneMesh {
  IMPLICIT_MESH_TYPE type;
  glm::vec4 data1;
  SceneMaterial material;
};

class Scene {
public:
  Scene() = default;
  ~Scene() = default;
  void loadSceneFromDescription(const std::string &path);

public:
  std::vector<ImplicitSceneMesh> m_implicitMeshes;
  void processPolygonShape(const nlohmann::json &jobj);
  void processImplicitShape(const nlohmann::json &jobj);
  void processImplicitSphere(const nlohmann::json &jobj);
  void processImplicitPlane(const nlohmann::json &jobj);
  SceneMaterial processSceneMaterial(const nlohmann::json &jobj);
};
} // namespace core
} // namespace mg_ray
