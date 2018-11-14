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

void CPURenderContext::run() {
  int w = m_settings->width;
  int h = m_settings->height;
  float *const pixels = m_data.get();
	for (int y = 0; y < h; ++y) {
	  for (int x = 0; x < w; ++x) {
      int id = (y * w  + x ) * 4;

      pixels[id + 0] = float(x) / float(w);
      pixels[id + 1] = float(y) / float(h);

      //pixels[id + 0] = 0.0f;
      //pixels[id + 1] = 0.0f;
      pixels[id + 2] = 0.0f;
      pixels[id + 3] = 1.0f;
    }
  }
  
}

void CPURenderContext::cleanup() {}
TextureOutput CPURenderContext::getTextureOutput() {
  return TextureOutput{TextureOutputType::CPU, m_settings->width,
                       m_settings->height, m_data.get()
  };
}
} // namespace core
} // namespace mg_ray
