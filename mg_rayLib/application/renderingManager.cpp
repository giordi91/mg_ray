#include "renderingManager.h"
#include "mg_rayLib/rendering/debugRenderer.h"
namespace mg_ray {
namespace application {
bool RenderingManager::initialize() {

  m_debugRenderer = new rendering::DebugRenderer();
  m_debugRenderer->initialize(&m_settings);

  return true;
}
void RenderingManager::run() {
  while (true) {
  }
}
} // namespace application
} // namespace mg_ray
