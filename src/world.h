#pragma once
#include "hitable.h"
#include <glm/glm.hpp>
#include <vector>

using glm::vec3;

struct Image {
  int width;
  int height;
  char *data;
};


// contains everything to be rendered in the scene
struct World {
  void rayCast(Image img, int samplePerPixel);

  std::vector<Hitable*> m_hitable;
};

