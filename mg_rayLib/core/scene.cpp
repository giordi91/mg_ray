#include "mg_rayLib/core/scene.h"
#include "mg_rayLib/core/file_utils.h"

#define ASSERT_VALUE_IN_JSON(jobj, key)                                        \
  if (jobj.find(key) == jobj.end()) {                                          \
    assert(0 && "key not found in json");                                      \
  }

#define NAME_TO_SHAPE_TYPE(name, returnValue)                                  \
  auto found = m_nameToShapeType.find(name);                                   \
  if (found == m_nameToShapeType.end()) {                                      \
    returnValue = SHAPE_TYPE::INVALID;                                         \
  } else {                                                                     \
    returnValue = found->second;                                               \
  }                                                                            \
  assert(returnValue != SHAPE_TYPE::INVALID)

#define NAME_TO_IMPLICIT_MESH_TYPE(jobj, returnValue)                          \
  const std::string stype =                                                    \
      get_value_if_in_json(jobj, SCENE_KEY_TYPE, DEFAULT_STRING);              \
  assert(!stype.empty() &&                                                     \
         "could not find type in shape being read from file");                 \
  auto found = m_nameToImplicitType.find(stype);                               \
  if (found == m_nameToImplicitType.end()) {                                   \
    returnValue = IMPLICIT_MESH_TYPE::INVALID;                                 \
  } else {                                                                     \
    returnValue = found->second;                                               \
  }

#define NAME_TO_MATERIAL_TYPE(jobj, returnValue)                               \
  const std::string stype =                                                    \
      get_value_if_in_json(jobj, SCENE_KEY_TYPE, DEFAULT_STRING);              \
  assert(!stype.empty() &&                                                     \
         "could not find type in material being read from file");              \
  auto found = m_nameToMaterialType.find(stype);                               \
  if (found == m_nameToMaterialType.end()) {                                   \
    returnValue = MATERIAL_TYPE::INVALID;                                      \
  } else {                                                                     \
    returnValue = found->second;                                               \
  }

template <>
inline mg_ray::core::DataFloat4
get_value_if_in_json(const nlohmann::json &data, std::string key,
                     mg_ray::core::DataFloat4 default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    return mg_ray::core::DataFloat4{vec[0].get<float>(), vec[1].get<float>(),
                                    vec[2].get<float>(), vec[3].get<float>()};
  }
  return default_value;
}

template <>
inline mg_ray::core::DataFloat3
get_value_if_in_json(const nlohmann::json &data, std::string key,
                     mg_ray::core::DataFloat3 default_value) {
  if (data.find(key) != data.end()) {
    auto &vec = data[key];
    return mg_ray::core::DataFloat3{vec[0].get<float>(), vec[1].get<float>(),
                                    vec[2].get<float>()};
  }
  return default_value;
}
namespace mg_ray {
namespace core {

const std::string Scene::SCENE_KEY_SHAPES = "shapes";
const std::string Scene::SCENE_KEY_TYPE = "type";
const std::string Scene::SCENE_KEY_MATERIAL = "material";
const std::string Scene::SCENE_KEY_ALBEDO = "albedo";
const std::string Scene::SCENE_KEY_GLOSSINESS = "glossiness";
const std::string Scene::SCENE_KEY_POSITION = "position";
const std::string Scene::SCENE_KEY_RADIUS = "radius";
const std::string Scene::SCENE_KEY_NORMAL = "normal";
const std::string Scene::SCENE_KEY_IMPLICIT_DATA = "implicitData";
const std::string Scene::DEFAULT_STRING;
const DataFloat4 Scene::DEFAULT_DATAFLOAT4{0.0f, 0.0f, 0.0f, 0.0f};
const DataFloat3 Scene::DEFAULT_DATAFLOAT3{0.0f, 0.0f, 0.0f};

const std::unordered_map<std::string, SHAPE_TYPE> Scene::m_nameToShapeType{
    {"implicit", SHAPE_TYPE::IMPLICIT}, {"polygons", SHAPE_TYPE::POLYGONS}};
const std::unordered_map<std::string, IMPLICIT_MESH_TYPE>
    Scene::m_nameToImplicitType{{"sphere", IMPLICIT_MESH_TYPE::SPHERE},
                         {"plane", IMPLICIT_MESH_TYPE::PLANE}};
const std::unordered_map<std::string, MATERIAL_TYPE>
    Scene::m_nameToMaterialType{{"diffuse", MATERIAL_TYPE::DIFFUSE},
                         {"metal", MATERIAL_TYPE::METAL},
                         {"dialectric", MATERIAL_TYPE::DIALECTRIC}};

void Scene::loadSceneFromDescription(const std::string &path) {

  auto jobj = get_json_obj(path);
  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_SHAPES);

  // getting the array of shapes
  auto shapesj = jobj[SCENE_KEY_SHAPES];

  for (const auto &shape : shapesj) {
    const std::string stype =
        get_value_if_in_json(shape, SCENE_KEY_TYPE, DEFAULT_STRING);
    assert(!stype.empty() &&
           "could not find type in shape being read from file");

    // extracting and processing the type of shape we have
    SHAPE_TYPE type;
    NAME_TO_SHAPE_TYPE(stype, type);
    switch (type) {
    case (SHAPE_TYPE::IMPLICIT): {
      processImplicitShape(shape);
      break;
    }
    case (SHAPE_TYPE::POLYGONS): {
      processPolygonShape(shape);
      break;
    }
    }
  }
}

void Scene::processImplicitShape(const nlohmann::json &jobj) {

  IMPLICIT_MESH_TYPE type;
  NAME_TO_IMPLICIT_MESH_TYPE(jobj, type);
  switch (type) {
  case (IMPLICIT_MESH_TYPE::SPHERE): {
    processImplicitSphere(jobj);
    break;
  }
  case (IMPLICIT_MESH_TYPE::PLANE): {
    processImplicitPlane(jobj);
    break;
  }
  }
}

SceneMaterial Scene::processSceneMaterial(const nlohmann::json &jobj) {

  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_ALBEDO);
  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_GLOSSINESS);
  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_TYPE);

  DataFloat3 albedo =
      get_value_if_in_json(jobj, SCENE_KEY_ALBEDO, DEFAULT_DATAFLOAT3);
  float glossiness = jobj[SCENE_KEY_GLOSSINESS].get<float>();
  MATERIAL_TYPE type;
  NAME_TO_MATERIAL_TYPE(jobj, type);
  return SceneMaterial{albedo, glossiness, type};
}
void Scene::processImplicitSphere(const nlohmann::json &jobj) {

  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_MATERIAL);
  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_IMPLICIT_DATA);

  auto impJ = jobj[SCENE_KEY_IMPLICIT_DATA];
  ASSERT_VALUE_IN_JSON(impJ, SCENE_KEY_POSITION);
  ASSERT_VALUE_IN_JSON(impJ, SCENE_KEY_RADIUS);

  auto pos = impJ[SCENE_KEY_POSITION];
  auto radius = impJ[SCENE_KEY_RADIUS];
  m_implicitMeshes.emplace_back(
      ImplicitSceneMesh{IMPLICIT_MESH_TYPE::SPHERE,
                        {pos[0].get<float>(), pos[1].get<float>(),
                         pos[2].get<float>(), radius[0].get<float>()},
                        processSceneMaterial(jobj)});
}

void Scene::processImplicitPlane(const nlohmann::json &jobj) {

  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_MATERIAL);
  ASSERT_VALUE_IN_JSON(jobj, SCENE_KEY_IMPLICIT_DATA);

  auto impJ = jobj[SCENE_KEY_IMPLICIT_DATA];
  ASSERT_VALUE_IN_JSON(impJ, SCENE_KEY_POSITION);
  ASSERT_VALUE_IN_JSON(impJ, SCENE_KEY_NORMAL);

  auto pos = impJ[SCENE_KEY_POSITION];
  auto normal = impJ[SCENE_KEY_NORMAL];
  m_implicitMeshes.emplace_back(
      ImplicitSceneMesh{IMPLICIT_MESH_TYPE::PLANE,
                        {normal[0].get<float>(), normal[1].get<float>(),
                         normal[2].get<float>(), pos[0].get<float>()},
                        processSceneMaterial(jobj)});
}
void Scene::processPolygonShape(const nlohmann::json &jobj) {
  assert(0 && "processing of polygon shape not implemented yet");
}

} // namespace core
} // namespace mg_ray
