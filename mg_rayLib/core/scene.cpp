#include "mg_rayLib/core/scene.h"
#include "mg_rayLib/core/file_utils.h"
#include "middleware/stb_image.h"

namespace mg_ray {
namespace core {

namespace sceneKeys {
static const std::string SCENE_KEY_SHAPES = "shapes";
static const std::string SCENE_KEY_TYPE = "type";
static const std::string SCENE_KEY_MATERIAL = "material";
static const std::string SCENE_KEY_ALBEDO = "albedo";
static const std::string SCENE_KEY_GLOSSINESS = "glossiness";
static const std::string SCENE_KEY_POSITION = "position";
static const std::string SCENE_KEY_RADIUS = "radius";
static const std::string SCENE_KEY_NORMAL = "normal";
static const std::string SCENE_KEY_IMPLICIT_DATA = "implicitData";
static const std::string SCENE_KEY_TEXTURE_WIDTH = "width";
static const std::string SCENE_KEY_TEXTURE_HEIGHT = "height";
static const std::string SCENE_KEY_TEXTURE_PATH = "path";
static const std::string SCENE_KEY_TEXTURE_BG = "backgroundTexture";
static const std::string DEFAULT_STRING;
static const int DEFAULT_INT = -1;
static const glm::vec4 DEFAULT_VEC4{0.0f, 0.0f, 0.0f, 0.0f};
static const glm::vec3 DEFAULT_VEC3{0.0f, 0.0f, 0.0f};

static const std::unordered_map<std::string, SHAPE_TYPE> m_nameToShapeType{
    {"implicit", SHAPE_TYPE::IMPLICIT}, {"polygons", SHAPE_TYPE::POLYGONS}};
static const std::unordered_map<std::string, IMPLICIT_MESH_TYPE>
    m_nameToImplicitType{{"sphere", IMPLICIT_MESH_TYPE::SPHERE},
                         {"plane", IMPLICIT_MESH_TYPE::PLANE}};
static const std::unordered_map<std::string, MATERIAL_TYPE>
    m_nameToMaterialType{{"diffuse", MATERIAL_TYPE::DIFFUSE},
                         {"metal", MATERIAL_TYPE::METAL},
                         {"dielectric", MATERIAL_TYPE::DIALECTRIC}};

static const std::unordered_map<std::string, TEXTURE_TYPE> nameToTextureType{
    {"T_2D", TEXTURE_TYPE::T_2D}};
} // namespace sceneKeys

SHAPE_TYPE nameToShape(const std::string &name) {
  auto found = sceneKeys::m_nameToShapeType.find(name);
  SHAPE_TYPE toReturn = SHAPE_TYPE::INVALID;
  if (found != sceneKeys::m_nameToShapeType.end()) {
    return found->second;
  }
  assert(toReturn != SHAPE_TYPE::INVALID);
  return toReturn;
}

IMPLICIT_MESH_TYPE nameToImplicitMesh(const nlohmann::json &jobj) {
  auto returnValue = IMPLICIT_MESH_TYPE::INVALID;
  const std::string stype = get_value_if_in_json(
      jobj, sceneKeys::SCENE_KEY_TYPE, sceneKeys::DEFAULT_STRING);
  assert(!stype.empty() && "could not find type in shape being read from file");
  auto found = sceneKeys::m_nameToImplicitType.find(stype);
  if (found != sceneKeys::m_nameToImplicitType.end()) {
    returnValue = found->second;
  }
  assert(returnValue != IMPLICIT_MESH_TYPE::INVALID);
  return returnValue;
}

template <typename T>
T nameToType(const nlohmann::json &jobj,
             const std::unordered_map<std::string, T> &sourceMap) {
  auto returnValue = T::INVALID;
  const std::string stype = get_value_if_in_json(
      jobj, sceneKeys::SCENE_KEY_TYPE, sceneKeys::DEFAULT_STRING);
  assert(!stype.empty() && "could not find type key while parsing file");
  auto found = sourceMap.find(stype);
  if (found != sourceMap.end()) {
    returnValue = found->second;
  }
  assert(returnValue != T::INVALID);
  return returnValue;
}

inline TEXTURE_TYPE getTextureType(const nlohmann::json &jobj) {
  return nameToType(jobj, sceneKeys::nameToTextureType);
}

inline SceneTexture loadTextrue(const nlohmann::json &textureJ) {

  TEXTURE_TYPE texType = nameToType(textureJ, sceneKeys::nameToTextureType);
  const std::string path = get_value_if_in_json(
      textureJ, sceneKeys::SCENE_KEY_TEXTURE_PATH, sceneKeys::DEFAULT_STRING);

  assert(!path.empty() && "error loading texture, no path provided");
  // as of now only one texture type is supported
  int width = -1;
  int height = -1;
  unsigned char *data = nullptr;
  if (texType == TEXTURE_TYPE::T_2D) {
    data = stbi_load(path.c_str(), &width, &height, 0, STBI_rgb_alpha);
    assert(data != nullptr && "error reading texture from file");
  }
  assert(width != -1 && "error loading texture, width is not defined");
  assert(height != -1 && "error loading texture, height is not defined");

  return SceneTexture{texType, width, height, data};
}

inline SceneTexture loadBackgroundTexture(const nlohmann::json &jobj) {

  assertValueInJson(jobj, sceneKeys::SCENE_KEY_TEXTURE_BG);
  const auto &textureJ = jobj[sceneKeys::SCENE_KEY_TEXTURE_BG];
  return loadTextrue(textureJ);
}

MATERIAL_TYPE nameToMaterialType(const nlohmann::json &jobj) {
  auto returnValue = MATERIAL_TYPE::INVALID;
  const std::string stype = get_value_if_in_json(
      jobj, sceneKeys::SCENE_KEY_TYPE, sceneKeys::DEFAULT_STRING);
  assert(!stype.empty() &&
         "could not find type in material being read from file");
  auto found = sceneKeys::m_nameToMaterialType.find(stype);
  if (found != sceneKeys::m_nameToMaterialType.end()) {
    returnValue = found->second;
  }
  assert(returnValue != MATERIAL_TYPE::INVALID);
  return returnValue;
}
void Scene::loadSceneFromDescription(const std::string &path) {

  auto jobj = get_json_obj(path);
  assertValueInJson(jobj, sceneKeys::SCENE_KEY_SHAPES);

  // getting the array of shapes
  auto shapesj = jobj[sceneKeys::SCENE_KEY_SHAPES];

  for (const auto &shape : shapesj) {
    const std::string stype = get_value_if_in_json(
        shape, sceneKeys::SCENE_KEY_TYPE, sceneKeys::DEFAULT_STRING);
    assert(!stype.empty() &&
           "could not find type in shape being read from file");

    // extracting and processing the type of shape we have
    SHAPE_TYPE type = nameToShape(stype);
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

  // next taking care of the background
  bgTexture = loadBackgroundTexture(jobj);
  int x = 0;
}

void Scene::processImplicitShape(const nlohmann::json &jobj) {

  assertValueInJson(jobj, sceneKeys::SCENE_KEY_IMPLICIT_DATA);
  auto impDataj = jobj[sceneKeys::SCENE_KEY_IMPLICIT_DATA];
  IMPLICIT_MESH_TYPE type = nameToImplicitMesh(impDataj);
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

  assertValueInJson(jobj, sceneKeys::SCENE_KEY_ALBEDO);
  assertValueInJson(jobj, sceneKeys::SCENE_KEY_GLOSSINESS);
  assertValueInJson(jobj, sceneKeys::SCENE_KEY_TYPE);

  glm::vec3 albedo = get_value_if_in_json(jobj, sceneKeys::SCENE_KEY_ALBEDO,
                                          sceneKeys::DEFAULT_VEC3);
  float glossiness = jobj[sceneKeys::SCENE_KEY_GLOSSINESS].get<float>();
  MATERIAL_TYPE type = nameToMaterialType(jobj);
  return SceneMaterial{albedo, glossiness, type};
}
void Scene::processImplicitSphere(const nlohmann::json &jobj) {

  assertValueInJson(jobj, sceneKeys::SCENE_KEY_MATERIAL);
  assertValueInJson(jobj, sceneKeys::SCENE_KEY_IMPLICIT_DATA);

  auto impJ = jobj[sceneKeys::SCENE_KEY_IMPLICIT_DATA];
  assertValueInJson(impJ, sceneKeys::SCENE_KEY_POSITION);
  assertValueInJson(impJ, sceneKeys::SCENE_KEY_RADIUS);

  auto pos = impJ[sceneKeys::SCENE_KEY_POSITION];
  auto radius = impJ[sceneKeys::SCENE_KEY_RADIUS];
  m_implicitMeshes.emplace_back(ImplicitSceneMesh{
      IMPLICIT_MESH_TYPE::SPHERE,
      {pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>(),
       radius.get<float>()},
      processSceneMaterial(jobj[sceneKeys::SCENE_KEY_MATERIAL])});
}

void Scene::processImplicitPlane(const nlohmann::json &jobj) {

  assertValueInJson(jobj, sceneKeys::SCENE_KEY_MATERIAL);
  assertValueInJson(jobj, sceneKeys::SCENE_KEY_IMPLICIT_DATA);

  auto impJ = jobj[sceneKeys::SCENE_KEY_IMPLICIT_DATA];
  assertValueInJson(impJ, sceneKeys::SCENE_KEY_POSITION);
  assertValueInJson(impJ, sceneKeys::SCENE_KEY_NORMAL);

  auto pos = impJ[sceneKeys::SCENE_KEY_POSITION];
  auto normal = impJ[sceneKeys::SCENE_KEY_NORMAL];
  m_implicitMeshes.emplace_back(ImplicitSceneMesh{
      IMPLICIT_MESH_TYPE::PLANE,
      {normal[0].get<float>(), normal[1].get<float>(), normal[2].get<float>(),
       pos.get<float>()},
      processSceneMaterial(jobj[sceneKeys::SCENE_KEY_MATERIAL])});
}
void Scene::processPolygonShape(const nlohmann::json &jobj) {
  assert(0 && "processing of polygon shape not implemented yet");
}

} // namespace core
} // namespace mg_ray
