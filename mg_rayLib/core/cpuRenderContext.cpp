#include "cpuRenderContext.h"
#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/foundation/window.h"
#include "middleware/glm/glm.hpp"

namespace mg_ray {
namespace core {

#define PI 3.14159265358979323846f /* pi */
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
glm::vec3 randomInUnitDisc(LCG &rnd) {
  glm::vec3 p;
  do {
    p = 2.0f * rnd.nextVec3() - glm::vec3(1.0f, 1.0f, 1.0f);
  } while (glm::dot(p, p) >= 1.0f);
  return p;
}

void getRayInSubPixelWithThinLens(int x, int y, glm::vec3 &pt, glm::vec3 &ray,
                                  const SceneCamera *camera,
                                  const GlobalSettings *settings, LCG &rnd) {

  float rowF = ((float)y + rnd.next()) / (float)settings->height;
  float colF = ((float)x + rnd.next()) / (float)settings->width;

  float lensRadius = camera->aperture * 0.5f;

  // compute the camera ray
  float theta = (float)(camera->vFov * 3.14 / 180.0);
  float halfHeight = (float)tanf(theta * 0.5);
  float halfWidth = halfHeight * (settings->width / settings->height);

  const float *v = &camera->view[0].x;
  glm::vec3 view{v[8], v[9], v[10]};
  glm::vec3 up{v[4], v[5], v[6]};
  glm::vec3 cross{v[0], v[1], v[2]};
  glm::vec3 pos{v[12], v[13], v[14]};

  float focusDist = camera->focusDistance;
  glm::vec3 lowerLeft = (pos - up * halfHeight * focusDist) -
                        (cross * halfWidth * focusDist - view * focusDist);
  glm::vec3 vertical = up * (2.0f * halfHeight * rowF) * focusDist;
  glm::vec3 horizontal = (cross * 2.0f * halfWidth * colF) * focusDist;

  // float3 lowerLeft = pos - up * halfHeight*focusDist - cross * halfWidth
  // *focusDist - view*focusDist; float3 vertical = up * (2.0f * halfHeight *
  // rowF)*focusDist; float3 horizontal = (cross * 2.0f * halfWidth *
  // colF)*focusDist;

  glm::vec3 rd = lensRadius * randomInUnitDisc(rnd);
  glm::vec3 offset;
  offset.x = rowF * rd.x;
  offset.y = colF * rd.y;
  offset.z = 0.0f;

  ray = lowerLeft + (vertical + horizontal) - pos - offset;
  ray = glm::normalize(ray);
  pt = pos + offset;
}

void getRayInSubPixel(int x, int y, float aperture, float focusDist,
                      glm::vec3 &point, glm::vec3 &ray,
                      const SceneCamera *camera, const GlobalSettings *settings,
                      LCG &rnd) {
  float rowF = ((float)y + rnd.next()) / (float)settings->height;
  float colF = ((float)x + rnd.next()) / (float)settings->width;

  float lensRadius = aperture * 0.5f;

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

  glm::vec3 rd = lensRadius * randomInUnitDisc(rnd);
  glm::vec3 offset;
  offset.x = rowF * rd.x;
  offset.y = colF * rd.y;
  offset.z = 0.0f;

  ray = lowerLeft + (vertical + horizontal) - pos - offset;
  ray = glm::normalize(ray);
  point = pos + offset;
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

inline glm::vec3 sampleBGTexture(int x, int y, const SceneTexture &tex) {
  int id = (y * tex.width + x) * 4;
  float INV_255 = 1.0f / 255.0f;
  float r = static_cast<float>(tex.data[id + 0]) * INV_255;
  float g = static_cast<float>(tex.data[id + 1]) * INV_255;
  float b = static_cast<float>(tex.data[id + 2]) * INV_255;
  return glm::vec3{r, g, b};
}

glm::vec3 randomInUnitSphere(LCG &rnd) {
  glm::vec3 p;
  do {
    p = 2.0f * rnd.nextVec3() - glm::vec3(1.0f, 1.0f, 1.0f);
  } while (dot(p, p) >= 1.0f);
  return p;
}

inline glm::vec3 reflect(const glm::vec3 I, glm::vec3 N) {
  return I - 2.0f * glm::dot(I, N) * N;
}

inline bool refract(const glm::vec3 I, const glm::vec3 N, float ior,
                    glm::vec3 &refract) {
  float dt = glm::clamp(glm::dot(glm::normalize(I), N), -1.0f, 1.0f);
  float discriminant = 1.0f - ior * ior * (1.0f - dt * dt);
  if (discriminant > 0.0f) {
    refract = ior * (glm::normalize(I) - N * dt) - N * glm::sqrt(discriminant);
    return true;
  } else {
    return false;
  }
}
inline float schlick(float cosine, float refIdx) {
  float r0 = (1.0f - refIdx) / (1.0f + refIdx);
  r0 = r0 * r0;
  return r0 + (1.0f - r0) * glm::pow((1.0f - cosine), 5);
}

void scatterMaterial(const glm::vec3 &inPos, const glm::vec3 &inRay,
                     HitRecord *rec, glm::vec3 &outPos, glm::vec3 &outRay,
                     glm::vec3 &attenuation, LCG &rnd,
                     ImplicitSceneMesh *meshes) {
  switch (meshes[rec->hitIndex].material.type) {
  case (MATERIAL_TYPE::DIFFUSE): {
    outPos = rec->position;
    outRay = glm::normalize(rec->normal + randomInUnitSphere(rnd));
    attenuation = meshes[rec->hitIndex].material.albedo;
    break;
  }
  case (MATERIAL_TYPE::METAL): {
    outPos = rec->position;
    outRay = reflect(inRay, rec->normal) +
             meshes[rec->hitIndex].material.roughness * randomInUnitSphere(rnd);
    attenuation = meshes[rec->hitIndex].material.albedo;
    break;
  }
  case (MATERIAL_TYPE::DIALECTRIC): {

    float refIdx = 1.5f;
    glm::vec3 outwardNormal;
    glm::vec3 reflected = reflect(inRay, rec->normal);
    attenuation = glm::vec3(1.0f, 1.0f, 1.0f);
    float ior;
    float cosine;
    float reflectProb;

    glm::vec3 refracted;

    if (glm::dot(inRay, rec->normal) > 0) {
      outwardNormal = -rec->normal;
      ior = refIdx;
      cosine = refIdx * glm::dot(inRay, rec->normal) / glm::length(inRay);
    } else {
      outwardNormal = rec->normal;
      ior = 1.0f / refIdx;
      cosine = -glm::dot(inRay, rec->normal) / glm::length(inRay);
    }

    if (refract(inRay, outwardNormal, ior, refracted)) {
      reflectProb = schlick(cosine, refIdx);
    } else {
      reflectProb = 1.0f;
      outPos = rec->position;
      outRay = reflected;
    }

    if (rnd.next() < reflectProb) {
      outPos = rec->position;
      outRay = reflected;
    } else {
      outPos = rec->position;
      outRay = refracted;
    }
    break;
  }
  default: {
    assert(0);
  }
  }
}
void CPURenderContext::run() {
  int w = m_settings->width;
  int h = m_settings->height;
  float *const pixels = m_data.get();

  int SPP = 1;
  int maxRecursion = 10;
  float t_min = 0.001f;
  float t_max = 1000.0f;

#pragma omp parallel for
  for (int y = 0; y < h; ++y) {
    LCG rnd(y, 324);
    glm::vec3 p;
    glm::vec3 ray;
    glm::vec3 posNext;
    glm::vec3 rayNext;
    glm::vec3 attenuation;
    glm::vec3 newAtt;
    glm::vec3 color;

    HitRecord rec;
    for (int x = 0; x < w; ++x) {

      int id = (y * w + x) * 4;
      color = glm::vec3{0.0f, 0.0f, 0.0f};

      for (int s = 0; s < SPP; ++s) {
        glm::vec3 attenuation{0.0f, 0.0f, 0.0f};

        // getRay(x, y, p, ray, &m_camera, m_settings);
        // getRayInSubPixel(x, y, 0.0f, 0.0f, p, ray, &m_camera, m_settings,
        // rnd);
        getRayInSubPixelWithThinLens(x, y, p, ray, &m_camera, m_settings, rnd);

        trace(t_min, t_max, p, ray, m_scene->m_implicitMeshes.data(),
              m_scene->m_implicitMeshes.size(), rec);

        if (rec.hitIndex >= 0) {
          scatterMaterial(p, ray, &rec, posNext, rayNext, attenuation, rnd,
                          m_scene->m_implicitMeshes.data());

          // checking if we hit directly a light
          if (m_scene->m_implicitMeshes[rec.hitIndex].material.type ==
              MATERIAL_TYPE::LIGHT) {
            color += m_scene->m_implicitMeshes[rec.hitIndex].material.albedo;
            continue;
          }

          for (int r = 0; r < maxRecursion; ++r) {

            // we shoot a ray based on how the material scattered
            trace(t_min, t_max, posNext, rayNext,
                  m_scene->m_implicitMeshes.data(),
                  m_scene->m_implicitMeshes.size(), rec);
            if (rec.hitIndex >= 0) {
              // lets scatter
              p = posNext;
              ray = rayNext;

              // checking if we hit directly a light
              if (m_scene->m_implicitMeshes[rec.hitIndex].material.type ==
                  MATERIAL_TYPE::LIGHT) {
                attenuation *=
                    m_scene->m_implicitMeshes[rec.hitIndex].material.albedo;
                break;
              }

              scatterMaterial(p, ray, &rec, posNext, rayNext, newAtt, rnd,
                              m_scene->m_implicitMeshes.data());
              attenuation *= newAtt;
              continue;
            } else {
              glm::vec3 bgColor = sampleBGTexture(x, y, m_scene->bgTexture);
              attenuation *= bgColor;
              break;
            }
          }

        } else {
          glm::vec3 bgColor = sampleBGTexture(x, y, m_scene->bgTexture);
          attenuation = bgColor;
        }
        color += attenuation;
      } // SPP

      color *= (1.0f / SPP);
      pixels[id + 0] = color.x;
      pixels[id + 1] = color.y;
      pixels[id + 2] = color.z;

    } // column
  }   // rows
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
