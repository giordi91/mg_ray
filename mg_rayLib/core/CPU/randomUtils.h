#pragma once

namespace mg_ray {
namespace core {

// Pseudo random number generator coming from the
// DXR Siggraph course
struct PseudoRandom {
private:
  static const unsigned int backoff = 16;
  int internalSeed;

public:
  PseudoRandom(int val0, int val1) {
    unsigned int v0 = val0, v1 = val1, s0 = 0;

    for (unsigned int n = 0; n < backoff; n++) {
      s0 += 0x9e3779b9;
      v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
      v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    internalSeed = v0;
  }
  float next() {

    internalSeed = (1664525u * internalSeed + 1013904223u);
    return float(internalSeed & 0x00FFFFFF) / float(0x01000000);
  }

  glm::vec3 nextVec3() { return glm::vec3(next(), next(), next()); }
};

// TODO fix random in disc
glm::vec3 randomInUnitDisc(PseudoRandom &rnd) {
  glm::vec3 p;
  do {
    p = 2.0f * rnd.nextVec3() - glm::vec3(1.0f, 1.0f, 1.0f);
  } while (glm::dot(p, p) >= 1.0f);
  return p;
}

glm::vec3 randomInUnitSphere(PseudoRandom &rnd) {
  glm::vec3 p;
  do {
    p = 2.0f * rnd.nextVec3() - glm::vec3(1.0f, 1.0f, 1.0f);
  } while (dot(p, p) >= 1.0f);
  return p;
}
} // namespace core
} // namespace mg_ray
