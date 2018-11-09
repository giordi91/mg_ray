#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "glm/glm.hpp"
#include "hitable.cpp"
#include "hitable.h"
#include "stb_image_write.h"
#include "threading.h"
#include "world.cpp"
#include "world.h"

constexpr int RGBA_C = 4;
constexpr int WIDTH_C = 1280;
constexpr int HEIGHT_C = 720;
using glm::vec3;

int main() {
  int arraySize = WIDTH_C * HEIGHT_C * 4;
  char *data = new char[arraySize];
  memset(data, 0, arraySize);

  Image img{WIDTH_C, HEIGHT_C, data};
  World world;
  Material pMaterial{ {0.2f,0.4f,0.3f} };
  Material sMaterial1{ {1.0f,0.0f,0.0f} };
  Material sMaterial2{ {0.2f,1.0f,0.0f} };
  Material sMaterial3{ {0.4f,0.3f,1.0f} };
  Material sMaterial4{ {0.8f,0.8f,0.0f} };

  //building the scene
  Plane p;
  p.normal = vec3(0, 1, 0);
  p.d = 0;
  p.mat = &pMaterial;
  Sphere s1;
  s1.position = vec3(2, 2, -2);
  s1.radius = 1.0f;
  s1.mat = &sMaterial1;
  Sphere s2;
  s2.position = vec3(0, 3, -1);
  s2.radius = 0.8f;
  s2.mat = &sMaterial2;
  Sphere s3;
  s3.position = vec3(-3.0f, -0.2, 0.0f);
  s3.radius = 2.0f;
  s3.mat = &sMaterial3;
  Sphere s4;
  s4.position = vec3(0, 0, -9);
  s4.radius = 5.0f;
  s4.mat = &sMaterial4;

  world.m_hitable.push_back(&p);
  world.m_hitable.push_back(&s1);
  world.m_hitable.push_back(&s2);
  world.m_hitable.push_back(&s3);
  world.m_hitable.push_back(&s4);

  int samplePerPixel = 4;
  world.rayCast(img, samplePerPixel);

  //writing the image
  stbi_write_bmp("test.bmp", WIDTH_C, HEIGHT_C, RGBA_C, data);
  delete data;

  return 0;
}
