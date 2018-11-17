#include "cpuRenderContext.h"
#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/foundation/window.h"
#include "middleware/glm/glm.hpp"

namespace mg_ray {
namespace core {

bool CPURenderContext::initialize(GlobalSettings *settings) {

  m_settings = settings;
  // lets allocate the resource
  m_data = std::unique_ptr<float[]>(
      new float[m_settings->width * m_settings->height * 4]);

  return true;
}

bool CPURenderContext::loadScene(Scene *scene) {
  // as of now, the high level definition is enough for us to work
  // with, we only use implicit scenes no need to worry about anything else
  m_scene = scene;
  return true;
}

void getRay(int x, int y, glm::vec3 &point, glm::vec3 &ray,
            const SceneCamera *camera, const GlobalSettings *settings) {

  float rowF = ((float)y) / (float)settings->height;
  float colF = ((float)x) / (float)settings->width;
  // compute the camera ray
  float theta = (float)(camera->vFov * 3.14 / 180.0);
  float halfHeight = (float)tanf(theta * 0.5);
  float halfWidth = halfHeight * (settings->width / settings->height);

  const float *v = &camera->view[0].x;
  glm::vec3 view{v[8], v[9], v[10]};
  glm::vec3 up{v[4], v[5], v[6]};
  glm::vec3 cross{v[0], v[1], v[2]};
  glm::vec3 pos{v[12], v[13], v[14]};

  glm::vec3 lowerLeft = (pos - up * halfHeight) - (cross * halfWidth - view);
  glm::vec3 vertical = up * (2.0f * halfHeight * rowF);
  glm::vec3 horizontal = cross * 2.0f * halfWidth * colF;
  ray = lowerLeft + (vertical + horizontal) - pos;
  ray = glm::normalize(ray);
  point = pos;
}

struct HitRecord {
  glm::vec3 normal;
  float t;
  glm::vec3 position;
  int hitIndex;
};

bool hitSphere(const glm::vec3 &center, float radius, const glm::vec3 &origin,
               const glm::vec3 &direction, float t_min, float t_max,
               HitRecord *rec) {
  glm::vec3 oc = origin - center;

  float a = glm::dot(direction, direction);
  float b = glm::dot(oc, direction);
  float c = glm::dot(oc, oc) - radius * radius;
  float discriminant = b * b - a * c;
  if (discriminant > 0) {
    float temp = (-b - sqrtf(discriminant)) / a;
    if (temp < t_max && temp > t_min) {
      rec->t = temp;
      rec->position = origin + direction * temp;
      rec->normal = (rec->position - center) / radius;
      return true;
    }
    temp = (-b + sqrtf(discriminant)) / a;
    if (temp < t_max && temp > t_min) {
      rec->t = temp;
      rec->position = origin + direction * temp;
      rec->normal = (rec->position - center) / radius;
      return true;
    }
  }
  return false;
}

bool hitPlane(const glm::vec3 &n, const glm::vec3 &planePoint,
              const glm::vec3 &origin, const glm::vec3 &direction, float t_min,
              float t_max, HitRecord *rec) {
  float denom = glm::dot(n, direction);
  if (glm::abs(denom) > 0.0001f) // your favorite epsilon
  {
    float t = glm::dot((planePoint - origin), n) / denom;
    if (t >= t_min && t < t_max) {
      rec->t = t;
      rec->position = origin + direction * t;
      rec->normal = n;
      return true; // you might want to allow an epsilon here too
    }
  }
  return false;
}

// tracing the scene
void trace(float t_min, float t_max, const glm::vec3 &point,
           const glm::vec3 &ray, const ImplicitSceneMesh *implicitData,
           int impLen, HitRecord &finalRec) {

  finalRec.hitIndex = -1;
  finalRec.t = t_max;
  HitRecord currentRec{};

  for (int impIdx = 0; impIdx < impLen; ++impIdx) {
    IMPLICIT_MESH_TYPE type = implicitData[impIdx].type;
    if (type == IMPLICIT_MESH_TYPE::SPHERE) {
      glm::vec3 impPos =
          glm::vec3{implicitData[impIdx].data1.x, implicitData[impIdx].data1.y,
                    implicitData[impIdx].data1.z};
      float radius = implicitData[impIdx].data1.w;
      bool hit =
          hitSphere(impPos, radius, point, ray, t_min, t_max, &currentRec);
      if (hit & (currentRec.t < finalRec.t)) {
        finalRec = currentRec;
        finalRec.hitIndex = impIdx;
      }
    } else {
      // means is a plane
      glm::vec3 impNorm =
          glm::vec3{implicitData[impIdx].data1.x, implicitData[impIdx].data1.y,
                    implicitData[impIdx].data1.z};
      float impPos = implicitData[impIdx].data1.w;
      bool hit = hitPlane(impNorm, impNorm * impPos, point, ray, t_min, t_max,
                          &currentRec);
      if (hit & (currentRec.t < finalRec.t)) {
        finalRec = currentRec;
        finalRec.hitIndex = impIdx;
      }
    }
  }
}

void CPURenderContext::run() {
  int w = m_settings->width;
  int h = m_settings->height;
  float *const pixels = m_data.get();

  glm::vec3 p;
  glm::vec3 ray;

  HitRecord rec;
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      int id = (y * w + x) * 4;
      getRay(x, y, p, ray, &m_camera, m_settings);

      float t_min = 0.001f;
      float t_max = 1000.0f;
      trace(t_min, t_max, p, ray, m_scene->m_implicitMeshes.data(),
            m_scene->m_implicitMeshes.size(), rec);

      if (rec.hitIndex >= 0) {
        pixels[id + 0] =
            m_scene->m_implicitMeshes[rec.hitIndex].material.albedo.x;
        pixels[id + 1] =
            m_scene->m_implicitMeshes[rec.hitIndex].material.albedo.y;
        pixels[id + 2] =
            m_scene->m_implicitMeshes[rec.hitIndex].material.albedo.z;
      } else {
        pixels[id + 0] = 0.5f;
        pixels[id + 1] = 0.5f;
        pixels[id + 2] = 0.5f;
      }
    }
  }
}

void CPURenderContext::cleanup() {}
TextureOutput CPURenderContext::getTextureOutput() {
  return TextureOutput{TextureOutputType::CPU, m_settings->width,
                       m_settings->height, m_data.get()};
}
void CPURenderContext::setSceneCamera(SceneCamera *camera) {
  m_camera = *camera;
}
} // namespace core
} // namespace mg_ray
