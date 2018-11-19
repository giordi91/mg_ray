#pragma once
#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/core/renderContext.h"
#include <string>

namespace mg_ray {
namespace foundation {
class Window;
class Input;
}
namespace core {
class RenderingContext;
class Scene;
}
namespace rendering
{
	class DebugRenderer;
}
namespace application {

class RenderingManager {
public:
  RenderingManager(core::GlobalSettings &settings) : m_settings(settings){};
  // this is the real entry point, here we initialize everything needed
  bool initialize();
  void loadSceneFromFile(const std::string& path);
  void run();
private:
#ifdef WIN32
	void MSWindowsRenderLoop();
#endif
private:
	void handleRaytracing();

private:
  core::GlobalSettings m_settings;
  rendering::DebugRenderer* m_debugRenderer;
  foundation::Input* m_input;
  core::Scene* m_scene;
  core::RenderContext* m_context;
  bool m_shouldTriggerRaytraceRender = false;
  bool m_shouldClearRaytraceRender= false;
  bool m_renderButtonDown = false;
  bool m_clearScreenFromRenderDown = false;
  bool m_preRaytraceNotificationSent = false;





  // std::unique_ptr<core::RenderingContext> m_context = nullptr;
};
} // namespace application
} // namespace mg_ray
