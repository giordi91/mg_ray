#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

constexpr int RGBA_C = 4;
constexpr int WIDTH_C = 800;
constexpr int HEIGHT_C = 600;

int main() {
  int arraySize = WIDTH_C * HEIGHT_C * 4;
  uint8_t *data = new uint8_t[arraySize];
  memset(data, 0, arraySize);
  for (int h = 0; h < HEIGHT_C; ++h) {

    float r = float(h) / float(HEIGHT_C);
    for (int w = 0; w < WIDTH_C; ++w) {

      float g = float(w) / float(WIDTH_C);
      float b = 0.2f;
      data[(h * WIDTH_C + w) * 4 + 0] = 255.99f * r;
      data[(h * WIDTH_C + w) * 4 + 1] = 255.99f * g;
      data[(h * WIDTH_C + w) * 4 + 2] = 255.99f * b;
      data[(h * WIDTH_C + w) * 4 + 3] = 255;
    }
  }

  stbi_write_bmp("test.bmp", WIDTH_C, HEIGHT_C, RGBA_C, data);
  delete data;

  return 0;
}
