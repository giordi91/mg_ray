#pragma once
#include "mg_rayLib/core/globalSettings.h"
#include <memory>

namespace mg_ray {
namespace foundation {
class Window;
}
namespace core {
class RenderingContext;
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
  void run();
private:
#ifdef WIN32
	void MSWindowsRenderLoop();
#endif

private:
  core::GlobalSettings m_settings;
  rendering::DebugRenderer* m_debugRenderer;




  // std::unique_ptr<core::RenderingContext> m_context = nullptr;
};
} // namespace application
} // namespace mg_ray
