#include "mg_rayLib/core/CPU/cpuRenderContext.h"
#include "mg_rayLib/core/CPU/cameraRay.h"
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
  // if we have implicit meshes we process only those, we don't
  // handle the case of polygons and implicit shapes together
  if (m_scene->m_implicitMeshes.size() == 0) {
    // since no implicit shapes, we try to process polys
    // meshes comes in a PositionNormalUVBuffer layout,
    // for a total of 8 float each vertex, we need to do
    // two things, first extract the positions only and next
    // if multiple geos are provided we need to put them
    // in a single buffer

    // now that we have the data we can build the bvh
    // m_bvh.init(&m_scene->m_polygonMeshes[0]);
    m_bvh.initMulti(m_scene->m_polygonMeshes.data(),
                    m_scene->m_polygonMeshes.size());
  }
  return true;
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
           const glm::vec3 &ray, const SceneImplicitMesh *implicitData,
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
                     glm::vec3 &attenuation, PseudoRandom &rnd,
                     SceneImplicitMesh *meshes) {
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

void scatterMaterialPoly(const glm::vec3 &inPos, const glm::vec3 &inRay,
                         HitRecord *rec, glm::vec3 &outPos, glm::vec3 &outRay,
                         glm::vec3 &attenuation, PseudoRandom &rnd,
                         ScenePolygonMesh *meshes) {
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

void CPURenderContext::renderImplicit() {

  int w = m_settings->width;
  int h = m_settings->height;
  float *const pixels = m_data.get();

  int SPP = m_settings->SPP;
  int maxRecursion = m_settings->maxRecursion;
  float t_min = m_settings->tMin;
  float t_max = m_settings->tMax;
#pragma omp parallel for
  for (int y = 0; y < h; ++y) {
    PseudoRandom rnd(y, 324);
    glm::vec3 p;
    glm::vec3 ray;
    glm::vec3 posNext;
    glm::vec3 rayNext;
    glm::vec3 newAtt;

    HitRecord rec;
    for (int x = 0; x < w; ++x) {

      int id = (y * w + x) * 4;
      auto color = glm::vec3{0.0f, 0.0f, 0.0f};

      for (int s = 0; s < SPP; ++s) {
        glm::vec3 attenuation{0.0f, 0.0f, 0.0f};

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
glm::vec3 CPURenderContext::getPolygonNormal(float u, float v, int meshIdx,
                                             int triangleIdx) {

  const ScenePolygonMesh &mesh = m_scene->m_polygonMeshes[meshIdx];
  int id1 = (triangleIdx + 0) * 8;
  int id2 = (triangleIdx + 1) * 8;
  int id3 = (triangleIdx + 2) * 8;

  auto n1 = glm::vec3{mesh.triangles[id1 + 3], mesh.triangles[id1 + 4],
                      mesh.triangles[id1 + 5]};

  auto n2 = glm::vec3{mesh.triangles[id2 + 3], mesh.triangles[id2 + 4],
                      mesh.triangles[id2 + 5]};

  auto n3 = glm::vec3{mesh.triangles[id3 + 3], mesh.triangles[id3 + 4],
                      mesh.triangles[id3 + 5]};
  float w = 1.0f - u - v;
  return n1 * u + n2 * v + n3 * w;
}

void CPURenderContext::renderPolygons() {

  int w = m_settings->width;
  int h = m_settings->height;
  float *const pixels = m_data.get();

  int SPP = m_settings->SPP;
  int maxRecursion = m_settings->maxRecursion;
  float t_min = m_settings->tMin;
  float t_max = m_settings->tMax;
#pragma omp parallel for
  for (int y = 0; y < h; ++y) {
    PseudoRandom rnd(y, 324);
    glm::vec3 p;
    glm::vec3 ray;
    glm::vec3 posNext;
    glm::vec3 rayNext;
    glm::vec3 newAtt;
    glm::vec3 color;

    HitRecord rec;
    for (int x = 0; x < w; ++x) {
      int id = (y * w + x) * 4;
      color = glm::vec3{0.0f, 0.0f, 0.0f};
      for (int s = 0; s < SPP; ++s) {
        glm::vec3 attenuation{0.0f, 0.0f, 0.0f};

        getRayInSubPixelWithThinLens(x, y, p, ray, &m_camera, m_settings, rnd);
        float outT = -1.0f;
        unsigned int outFaceId = 0;
        float u, v;
        bool hit = m_bvh.intersect(p, ray, outT, outFaceId, u, v, false);
        if (hit) {
          // here we multiply by 3 because we are getting back
          // the triangle hit, internally the bvh needs to know
          // the vertex index, need to fix that
          int localTriangleIndex;
          int meshIdx = m_bvh.getMeshIndex(outFaceId * 3, localTriangleIndex);
          assert(meshIdx < m_scene->m_polygonMeshes.size());
          rec.hitIndex = meshIdx;
          rec.normal = getPolygonNormal(u, v, meshIdx, localTriangleIndex);
          rec.position = p + ray * outT;

          scatterMaterialPoly(p, ray, &rec, posNext, rayNext, attenuation, rnd,
                              m_scene->m_polygonMeshes.data());
          for (int i = 0; i < maxRecursion; ++i) {
            bool hit =
                m_bvh.intersect(posNext, rayNext, outT, outFaceId, u, v, false);
            if (hit) {
              p = posNext;
              ray = rayNext;
              glm::vec3 newAtt{};
              scatterMaterialPoly(p, ray, &rec, posNext, rayNext, newAtt,
                                  rnd, m_scene->m_polygonMeshes.data());
				attenuation *= newAtt;
				continue;
            }
			else
			{
				attenuation *= sampleBGTexture(x, y, m_scene->bgTexture);
				break;
			}
          }

          // color.x += m_scene->m_polygonMeshes[meshIdx].material.albedo.x;
          // color.y += m_scene->m_polygonMeshes[meshIdx].material.albedo.y;
          // color.z += m_scene->m_polygonMeshes[meshIdx].material.albedo.z;

          //color += rec.normal;
          // color.x += m_scene->m_polygonMeshes[meshIdx].material.albedo.x;
          // color.y += m_scene->m_polygonMeshes[meshIdx].material.albedo.y;
          // color.z += m_scene->m_polygonMeshes[meshIdx].material.albedo.z;
        } else {
          attenuation = sampleBGTexture(x, y, m_scene->bgTexture);
        }

		color += attenuation;
      }
      pixels[id + 0] = color.x / (float)SPP;
      pixels[id + 1] = color.y / (float)SPP;
      pixels[id + 2] = color.z / (float)SPP;
    }
  }
}

void CPURenderContext::run() {

  if (m_scene->m_implicitMeshes.size() > 0) {
    renderImplicit();
  } else {
    renderPolygons();
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
