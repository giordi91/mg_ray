#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "middleware/json/jsonForwadDeclare.hpp"

namespace mg_ray {
namespace core {
enum class IMPLICIT_MESH_TYPE { SPHERE, PLANE, INVALID };
enum class SHAPE_TYPE { IMPLICIT, POLYGONS, INVALID };
enum class MATERIAL_TYPE { DIFFUSE, METAL, DIALECTRIC, INVALID };

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

private:
  static const std::string SCENE_KEY_SHAPES;
  static const std::string SCENE_KEY_TYPE;
  static const std::string SCENE_KEY_MATERIAL;
  static const std::string SCENE_KEY_ALBEDO;
  static const std::string SCENE_KEY_GLOSSINESS;
  static const std::string SCENE_KEY_POSITION;
  static const std::string SCENE_KEY_RADIUS;
  static const std::string SCENE_KEY_NORMAL;
  static const std::string SCENE_KEY_IMPLICIT_DATA;
  static const std::string DEFAULT_STRING;
  static const DataFloat4 DEFAULT_DATAFLOAT4;
  static const DataFloat3 DEFAULT_DATAFLOAT3;
  static const std::unordered_map<std::string, SHAPE_TYPE> m_nameToShapeType;
  static const std::unordered_map<std::string, IMPLICIT_MESH_TYPE>
      m_nameToImplicitType;
  static const std::unordered_map<std::string, MATERIAL_TYPE>
      m_nameToMaterialType;
};
} // namespace core
} // namespace mg_ray
