#pragma once
#include <memory>

#include "mg_rayLib/core/renderContext.h"
#include "mg_rayLib/core/scene.h"

namespace mg_ray {

namespace foundation {
class Window;
}

namespace core {
struct LCG {
private:
	static const unsigned int backoff = 16;
	int internalSeed;

public:
  LCG(int val0, int val1) {  
  	unsigned int v0 = val0, v1 = val1, s0 = 0;

	for (unsigned int n = 0; n < backoff; n++)
	{
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	internalSeed = v0;
  }

  float next() {
    //_last = (((a * _last) + c) % m);
    //return (float)((double)_last / (double)m);
    //return static_cast<float>(rand() / (RAND_MAX + 1.));

	internalSeed = (1664525u * internalSeed + 1013904223u);
	return float(internalSeed & 0x00FFFFFF) / float(0x01000000);
  }

  glm::vec3 nextVec3() { return glm::vec3(next(), next(), next()); }
};

class CPURenderContext : public RenderContext {

public:
  CPURenderContext() : RenderContext(){};
  virtual ~CPURenderContext() = default;

  bool initialize(GlobalSettings *settings) override;
  bool loadScene(Scene *scene) override;
  void run() override;
  void cleanup() override;
  TextureOutput getTextureOutput() override;
  void setSceneCamera(SceneCamera *camera) override;

private:
  foundation::Window *m_window = nullptr;
  Scene *m_scene;
  GlobalSettings *m_settings;
  std::unique_ptr<float[]> m_data;
  SceneCamera m_camera;
};
} // namespace core
} // namespace mg_ray
