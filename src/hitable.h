#pragma once
#include <glm/glm.hpp>

using glm::vec3;

struct Material {
  vec3 color;
};

struct Hitable {
  virtual float rayCast(vec3 rayOrigin, vec3 rayDirection) const = 0;
  Material *mat = nullptr;
};

struct Plane : Hitable {
  Plane() = default;
  virtual ~Plane() = default;
  vec3 normal;
  float d;

  float rayCast(vec3 rayOrigin, vec3 rayDirection) const override;
};

struct Sphere : Hitable {

  Sphere() = default;
  virtual ~Sphere() = default;
  vec3 position;
  float radius;

  float rayCast(vec3 rayOrigin, vec3 rayDirection) const override;
};
