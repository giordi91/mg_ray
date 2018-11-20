#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "middleware/glm/glm.hpp"
#include "middleware/json/jsonForwadDeclare.hpp"

namespace mg_ray {
namespace core {

enum class SHAPE_TYPE { IMPLICIT, TRIANGLE_SOUP, INVALID };
enum class IMPLICIT_MESH_TYPE { SPHERE, PLANE, INVALID };
enum class MATERIAL_TYPE { DIFFUSE, METAL, DIALECTRIC, LIGHT, INVALID };
// suffix T is only for not having 2D as an enum
enum class TEXTURE_TYPE { T_2D, INVALID };

struct SceneCamera {
  glm::mat4x4 view;
  float vFov;
  float aperture;
  float focusDistance;
  float padding;
};

struct SceneMaterial {
  glm::vec3 albedo;
  float roughness;
  MATERIAL_TYPE type;
};

struct SceneImplicitMesh {
  IMPLICIT_MESH_TYPE type;
  glm::vec4 data1;
  SceneMaterial material;
};
struct ScenePolygonMesh {
  std::unique_ptr<float[]> triangles;
  unsigned int vertexCount;
  SceneMaterial material;
};

struct SceneTexture {
  TEXTURE_TYPE type;
  int width = -1;
  int height = -1;
  std::unique_ptr<unsigned char[]> data = nullptr;
};

class Scene {
public:
  Scene() = default;
  ~Scene() = default;
  void loadSceneFromDescription(const std::string &path);

public:
  std::vector<SceneImplicitMesh> m_implicitMeshes;
  std::vector<ScenePolygonMesh> m_polygonMeshes;
  SceneTexture bgTexture;

private:
  void processPolygonShape(const nlohmann::json &jobj);
  void processImplicitShape(const nlohmann::json &jobj);
  void processImplicitSphere(const nlohmann::json &jobj);
  void processImplicitPlane(const nlohmann::json &jobj);
  SceneMaterial processSceneMaterial(const nlohmann::json &jobj);
};
} // namespace core
} // namespace mg_ray
