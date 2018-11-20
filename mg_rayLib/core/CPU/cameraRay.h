#pragma once
#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/core/scene.h"
#include "mg_rayLib/core/CPU/randomUtils.h"
#include "middleware/glm/glm.hpp"

namespace mg_ray {
namespace core {
inline void getRay(int x, int y, glm::vec3 &point, glm::vec3 &ray,
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
} // namespace

inline void getRayInSubPixelWithThinLens(int x, int y, glm::vec3 &pt,
                                         glm::vec3 &ray,
                                         const SceneCamera *camera,
                                         const GlobalSettings *settings,
                                         PseudoRandom &rnd) {

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

  glm::vec3 rd = lensRadius * randomInUnitDisc(rnd);
  glm::vec3 offset;
  offset.x = rowF * rd.x;
  offset.y = colF * rd.y;
  offset.z = 0.0f;

  ray = lowerLeft + (vertical + horizontal) - pos - offset;
  ray = glm::normalize(ray);
  pt = pos + offset;
}

inline void getRayInSubPixel(int x, int y, float aperture, float focusDist,
                             glm::vec3 &point, glm::vec3 &ray,
                             const SceneCamera *camera,
                             const GlobalSettings *settings, PseudoRandom &rnd) {
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
} // namespace core
} // namespace mg_ray
