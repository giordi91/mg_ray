#include "cpuRenderContext.h"
#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/foundation/window.h"

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

inline void vec3Sub(float *a, float *b, float *c) {
  c[0] = a[0] - b[0];
  c[1] = a[1] - b[1];
  c[2] = a[2] - b[2];
}

inline void vec3SubInPlace(float *a, float *b) {
  a[0] -= b[0];
  a[1] -= b[1];
  a[2] -= b[2];
}
inline void vec3Scale(float *a, float scalar, float *c) {
  c[0] = a[0] * scalar;
  c[1] = a[1] * scalar;
  c[2] = a[2] * scalar;
}
inline void vec3ScaleInPlace(float *a, float scalar) {
  a[0] *= scalar;
  a[1] *= scalar;
  a[2] *= scalar;
}
inline void vec3AddInPlace(float *a, float *b) {
  a[0] += b[0];
  a[1] += b[1];
  a[2] += b[2];
}

inline void vec3Add(float *a, float *b, float *c) {
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}
inline void vec3Normalize(float *a, float *b) {
  float d = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
  float inv = 1.0f / d;
  b[0] = a[0] * inv;
  b[1] = a[1] * inv;
  b[2] = a[2] * inv;
}

inline void vec3Mad(float *a, float *b, float *c, float *d) {
  // d = a*b + c;
  d[0] = a[0] * b[0] + c[0];
  d[1] = a[1] * b[1] + c[1];
  d[2] = a[2] * b[2] + c[2];
}

inline float vec3Dot(float *a, float *b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void getRay(int x, int y, float *point, float *ray, SceneCamera *camera,
            GlobalSettings *settings) {

  float rowF = ((float)y) / (float)settings->height;
  float colF = ((float)x) / (float)settings->width;
  // compute the camera ray
  float theta = (float)(camera->vFov * 3.14 / 180.0);
  float halfHeight = (float)tanf(theta * 0.5);
  float halfWidth = halfHeight * (settings->width / settings->height);

  // TODO check if is better to use float4x4 and use swizzling
  float *view = camera->view + 8;
  // new float3(cameraMatrix[0, 2], cameraMatrix[1, 2], cameraMatrix[2, 2]);
  float *up = camera->view + 4;
  // new float3(cameraMatrix[0, 1], cameraMatrix[1, 1], cameraMatrix[2, 1]);
  float *cross = camera->view;
  // new float3(cameraMatrix[0, 0], cameraMatrix[1, 0], cameraMatrix[2, 0]);
  float *pos = camera->view + 12;
  //    new float3(cameraMatrix[0, 3], cameraMatrix[1, 3], cameraMatrix[2,
  //    3]);

  float temp[3];
  float tempT[3];
  float temp2[3];
  // vec3Sub(pos, up, temp);
  // vec3ScaleInPlace(temp, halfHeight);
  vec3Scale(up, halfHeight, temp);
  vec3Sub(pos, temp, tempT);

  vec3Scale(cross, halfWidth, temp2);
  vec3SubInPlace(temp2, view);
  float lowerLeft[3];
  vec3Sub(tempT, temp2, lowerLeft);

  float vertical[3];
  vec3Scale(up, 2.0f * halfHeight * rowF, vertical);

  float horizontal[3];
  vec3Scale(cross, 2.0f * halfWidth * colF, horizontal);

  float temp3[3] = {0.0f, 0.0f, 0.0f};

  vec3AddInPlace(temp3, lowerLeft);
  vec3AddInPlace(temp3, horizontal);
  vec3AddInPlace(temp3, vertical);
  vec3SubInPlace(temp3, pos);

  vec3Normalize(temp3, ray);

  // float3 lowerLeft = pos - up * halfHeight - cross * halfWidth - view;
  // float3 vertical = up * (2.0f * halfHeight * rowF);
  // float3 horizontal = cross * 2.0f * halfWidth * colF;
  // ray = lowerLeft + (vertical + horizontal) - pos;
  // ray = math.normalize(ray);
  // point = pos;
  // position
  point[0] = pos[0];
  point[1] = pos[1];
  point[2] = pos[2];
}

struct HitRecord {
  float normal[3];
  float t;
  float position[3];
  int hitIndex;
};

bool hitSphere(float *center, float radius, float *origin, float *direction,
               float t_min, float t_max, HitRecord *rec) {
  // float3 oc = origin - center;
  float oc[3];
  vec3Sub(origin, center, oc);

  float a = vec3Dot(direction, direction);
  float b = vec3Dot(oc, direction);
  float c = vec3Dot(oc, oc) - radius * radius;
  float discriminant = b * b - a * c;
  if (discriminant > 0) {
    float temp = (-b - sqrtf(discriminant)) / a;
    if (temp < t_max && temp > t_min) {
      rec->t = temp;
      // rec->position = origin + direction * temp;
      float intermediate[3];
      vec3Scale(direction, temp, intermediate);
      vec3Add(origin, intermediate, rec->position);

      // rec.normal = (rec.position - center) / radius;
      // rec.normal = (rec.position - center) / radius;
      vec3Sub(rec->position, center, intermediate);
      vec3Scale(intermediate, (1.0f / radius), rec->normal);
      return true;
    }
    temp = (-b + sqrtf(discriminant)) / a;
    if (temp < t_max && temp > t_min) {
      ////rec.t = temp;
      // rec.position = origin + direction * temp;
      // rec.normal = (rec.position - center) / radius;
      // return true;

      // rec->position = origin + direction * temp;
      float intermediate[3];
      vec3Scale(direction, temp, intermediate);
      vec3Add(origin, intermediate, rec->position);

      // rec.normal = (rec.position - center) / radius;
      // rec.normal = (rec.position - center) / radius;
      vec3Sub(rec->position, center, intermediate);
      vec3Scale(intermediate, (1.0f / radius), rec->normal);
      return true;
    }
  }
  return false;
}

void CPURenderContext::run() {
  int w = m_settings->width;
  int h = m_settings->height;
  float *const pixels = m_data.get();
  float p[3];
  float ray[3];
  HitRecord rec;
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      int id = (y * w + x) * 4;
      getRay(x, y, p, ray, &m_camera, m_settings);

      // pixels[id + 0] = ray[0];
      // pixels[id + 1] = ray[1];
      // pixels[id + 2] = ray[2];

      ImplicitSceneMesh &m = m_scene->m_implicitMeshes[0];
	  if (y == 300 && x == 600)
	  {
		  int  ddd= 0;
	  }


      bool hit =
          hitSphere(&m.data1.x, m.data1.w, p, ray, 0.001f, 1000.0f, &rec);
      if (hit) {
        pixels[id + 0] = 1.0f;
        pixels[id + 1] = 0.0f;
        pixels[id + 2] = 0.0f;
      } else {
        pixels[id + 0] = 0.0f;
        pixels[id + 1] = 1.0f;
        pixels[id + 2] = 0.0f;
      }

      //// pixels[id + 0] = 0.0f;
      //// pixels[id + 1] = 0.0f;
      pixels[id + 3] = 1.0f;
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
