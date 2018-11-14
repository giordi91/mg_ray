#pragma once
#include <memory>

#include "mg_rayLib/core/renderContext.h"
#include "mg_rayLib/core/scene.h"

namespace mg_ray {

namespace foundation {
class Window;
}

namespace core {

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
