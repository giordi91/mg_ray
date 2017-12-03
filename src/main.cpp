#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "glm/glm.hpp"
#include "stb_image_write.h"
#include "threading.h"

constexpr int RGBA_C = 4;
constexpr int WIDTH_C = 1280;
constexpr int HEIGHT_C = 720;
using glm::vec3;

class Ray {
public:
  Ray() = default;
  Ray(const glm::vec3 &origin, const glm::vec3 &direction)
      : m_origin(origin), m_direction(direction) {}
  glm::vec3 pointAtParameter(float t) const {
    return m_origin + t * m_direction;
  }
  glm::vec3 m_origin;
  glm::vec3 m_direction;
};

bool hit_sphere(const vec3 &center, float radius, const Ray &r) {
  vec3 oc = r.m_origin - center;
  float a = glm::dot(r.m_direction, r.m_direction);
  float b = 2.0f * glm::dot(oc, r.m_direction);
  float c = glm::dot(oc, oc) - (radius * radius);
  float discriminant = b * b - (4.0f * a * c);
  return discriminant > 0;
}

glm::vec3 color(const Ray &r) {
  if (hit_sphere(vec3(0.0f, 0.0f, -10.0f), 0.5f, r)) {
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }

  // if (hit_sphere(vec3(1.0f, 0.0f, -1.0f), 0.5f, r)) {
  //  return glm::vec3(1.0f, 0.0f, 0.0f);
  //}
  vec3 unit_direction = glm::normalize(r.m_direction);
  float t = 0.5f * (unit_direction.y + 1.0f);
  return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
}

struct Image {
  int width;
  int height;
  char *data;
};

struct Material {
  vec3 color;
};

struct Plane {
  vec3 normal;
  float d;
  int matIndex;
};

struct Sphere {
  vec3 position;
  float radius;
  int matIndex;
};

float tolerance = 0.00001f;
float intersectPlane(vec3 rayOrigin, vec3 rayDirection, vec3 normal, float d) {
  float denominator = glm::dot(normal, rayDirection);
  if (abs(denominator) < tolerance) {
    return 999999.0f;
  }
  return (-d - glm::dot(normal, rayOrigin)) / (denominator);
}
float intersectSphere(vec3 rayOrigin, vec3 rayDirection, vec3 position,
                      float radius) {

  vec3 sphereLocalOrigin = rayOrigin - position;
  //vec3 sphereLocalOrigin = rayOrigin ;
  float a = glm::dot(rayDirection, rayDirection);
  float b = 2.0f * glm::dot(rayDirection, sphereLocalOrigin);
  float c = glm::dot(sphereLocalOrigin, sphereLocalOrigin) - (radius * radius);
  float denom = 2.0f * a;
  float rootTerm = sqrt(b * b - (4.0f * a * c));
  if (abs(rootTerm) > tolerance) {
    float tp = (-b + rootTerm) / denom;
    float tn = (-b - rootTerm) / denom;
    float t = tp;
    if ((tn > 0) && (tn < tp)) {
      t = tn;
    }
    return t;

  } else {
    return 999999.0f;
  }
}

vec3 rayCast(vec3 rayOrigin, vec3 rayDirection) {
  vec3 result = vec3(0.0f, 0.0f, 0.0f);
  float hitDistance = 999999.0f;

  Plane p{vec3(0, 1, 0), 0};

  float thisDistance = intersectPlane(rayOrigin, rayDirection, p.normal, p.d);
  if ((thisDistance > 0) && (thisDistance < hitDistance)) {
    hitDistance = thisDistance;
    result = vec3(0.0f, 0.3f, 0.3f);
  }

  Sphere s{vec3(2, 2, -2), 1.0f};
  thisDistance = intersectSphere(rayOrigin, rayDirection, s.position, s.radius);
  if ((thisDistance > 0) && (thisDistance < hitDistance)) {
    hitDistance = thisDistance;
    result = vec3(1.0f, 0.0f, 0.0f);
  }

  return result;
}

int main() {
  int arraySize = WIDTH_C * HEIGHT_C * 4;

  char *data = new char[arraySize];
  memset(data, 0, arraySize);

  // glm::vec3 lowerLeftCorner{-2.0f, -1.0f, -1.0f};
  // glm::vec3 horizontal{4.0f, 0.0f, 0.0f};
  // glm::vec3 vertical{0.0f, 2.0f, 0.0f};
  // glm::vec3 origin{0.0f, 0.0f, 0.0f};

  vec3 camZ = glm::normalize(vec3(0.0f, 0.1f, 1.0f));
  vec3 up(0.0f, 1.0f, 0.0f);
  vec3 camX = glm::normalize(glm::cross(up, camZ));
  vec3 camY = glm::normalize(glm::cross(camZ, camX));
  vec3 camP(0.0f, 2.0f, 10.0f);
  float filmDist = 1.0f;
  vec3 filmCenter = camP + (filmDist * (-camZ));

  float ratio = float(HEIGHT_C) / float(WIDTH_C);
  float filmWidth = 1.0f;
  float filmHeight = filmWidth*ratio;
  float halfFilmWidth = 0.5f * filmWidth;
  float halfFilmHeight = 0.5f * filmHeight;

  for (int h = 0; h < HEIGHT_C; ++h) {
    float filmY = -1.0f + 2.0f * (float(h) / float(HEIGHT_C));

    for (int w = 0; w < WIDTH_C; ++w) {
      float filmX = -1.0f + 2.0f * (float(w) / float(WIDTH_C));

      vec3 filmP = filmCenter + (filmX * halfFilmWidth * camX) +
                   (filmY * halfFilmHeight * camY);
      // Ray r(camP, glm::normalize(filmP - camP));
      // glm::vec3 col = color(r);
      vec3 rayOrigin = camP;
      vec3 rayDirection = glm::normalize(filmP - camP);
      vec3 col = rayCast(rayOrigin, rayDirection);

      data[((HEIGHT_C - h - 1) * WIDTH_C + w) * 4 + 0] = 255.99f * col.x;
      data[((HEIGHT_C - h - 1) * WIDTH_C + w) * 4 + 1] = 255.99f * col.y;
      data[((HEIGHT_C - h - 1) * WIDTH_C + w) * 4 + 2] = 255.99f * col.z;
      data[((HEIGHT_C - h - 1) * WIDTH_C + w) * 4 + 3] = 255;
    }
  }

  stbi_write_bmp("test.bmp", WIDTH_C, HEIGHT_C, RGBA_C, data);
  delete data;

  return 0;
}
