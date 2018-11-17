#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "middleware/json/jsonForwadDeclare.hpp"
#include "middleware/glm/glm.hpp"

namespace mg_ray {
namespace core {

enum class IMPLICIT_MESH_TYPE { SPHERE, PLANE, INVALID };
enum class SHAPE_TYPE { IMPLICIT, POLYGONS, INVALID };
enum class MATERIAL_TYPE { DIFFUSE, METAL, DIALECTRIC, INVALID };


struct SceneCamera
{
	//float view[16];
	glm::mat4x4 view;
	float vFov;
	float aperture;
	float focusDistance;
	float padding;
};


struct DataFloat4 {
  float x, y, z, w;
};
struct DataFloat3 {
  float x, y, z;
};
struct SceneMaterial {
  DataFloat3 albedo;
  float glossiness;
  MATERIAL_TYPE type;
};

struct ImplicitSceneMesh {
  IMPLICIT_MESH_TYPE type;
  DataFloat4 data1;
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
