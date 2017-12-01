#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "glm/glm.hpp"
#include "stb_image_write.h"
#include "threading.h"

constexpr int RGBA_C = 4;
constexpr int WIDTH_C = 1000;
constexpr int HEIGHT_C = 500;
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
  if (hit_sphere(vec3(0.0f, 0.0f, -1.0f), 0.5f, r)) {
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }

  if (hit_sphere(vec3(1.0f, 0.0f, -1.0f), 0.5f, r)) {
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  vec3 unit_direction = glm::normalize(r.m_direction);
  float t = 0.5f * (unit_direction.y + 1.0f);
  return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
}

struct Image
{
	int width;
	int height;
	char* data;
};

struct Material
{
	vec3 color;
};

struct Plane
{
	vec3 normal;
	float d;
	int matIndex;
};

struct Sphere
{
	vec3 position;
	float radius;
	int matIndex;
};


int main() {
  int arraySize = WIDTH_C * HEIGHT_C * 4;

  char *data = new char[arraySize];
  memset(data, 0, arraySize);

  glm::vec3 lowerLeftCorner{-2.0f, -1.0f, -1.0f};
  glm::vec3 horizontal{4.0f, 0.0f, 0.0f};
  glm::vec3 vertical{0.0f, 2.0f, 0.0f};
  glm::vec3 origin{0.0f, 0.0f, 0.0f};
  for (int h = 0; h < HEIGHT_C; ++h) {

    for (int w = 0; w < WIDTH_C; ++w) {

      float u = float(w) / float(WIDTH_C);
      float v = float(h) / float(HEIGHT_C);
      Ray r(origin, lowerLeftCorner + u * horizontal + v * vertical);
      glm::vec3 col = color(r);

      data[((HEIGHT_C-h-1) * WIDTH_C + w) * 4 + 0] = 255.99f * col.x;
      data[((HEIGHT_C-h-1) * WIDTH_C + w) * 4 + 1] = 255.99f * col.y;
      data[((HEIGHT_C-h-1) * WIDTH_C + w) * 4 + 2] = 255.99f * col.z;
      data[((HEIGHT_C-h-1) * WIDTH_C + w) * 4 + 3] = 255;
    }
  }

  stbi_write_bmp("test.bmp", WIDTH_C, HEIGHT_C, RGBA_C, data);
  delete data;

  return 0;
}
