#include "renderingManager.h"
#ifdef WIN32
#include "mg_rayLib/foundation/input.h"
#include "mg_rayLib/rendering/dxRenderer/dxDebugRenderer.h"
#endif

#include "mg_rayLib/core/cpuRenderContext.h"
#include "mg_rayLib/core/scene.h"
#include <iostream>
namespace mg_ray {
namespace application {
bool RenderingManager::initialize() {

  m_input = new foundation::Input();
#ifdef WIN32
  auto *winRenderer = new rendering::dx11::Dx11DebugRenderer();
  winRenderer->initialize(m_input, &m_settings);
  m_debugRenderer = winRenderer;
#else
  assert(0);
#endif

  // creating the render context
  switch (m_settings.hardwareType) {
  case (core::HardwareType::CPU): {
    m_context = new core::CPURenderContext();
    m_context->initialize(&m_settings);
  }
  }
  assert(m_context != nullptr && "no render context was created");
  return true;
}
void RenderingManager::run() {

#ifdef WIN32
  MSWindowsRenderLoop();
#endif
}
void RenderingManager::MSWindowsRenderLoop() {
  MSG msg;
  bool done = false;

  // initialize the message structure.
  ZeroMemory(&msg, sizeof(MSG));

  // Loop until there is a quit message from the window or the user.
  done = false;
  while (!done) {
    // Handle the windows messages.
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // If windows signals to end the application then exit out.
    if (msg.message == WM_QUIT) {
      done = true;
    } else {
      if (m_input->IsKeyDown(VK_ESCAPE)) {
        done = true;
        continue;
      }
      if (!m_input->IsKeyDown(VK_RETURN) && m_renderButtonDown) {
        m_shouldTriggerRaytraceRender = true;
      }
      m_renderButtonDown = m_input->IsKeyDown(VK_RETURN);

      if (!m_input->IsKeyDown(VK_BACK) && m_clearScreenFromRenderDown) {
        m_shouldClearRaytraceRender = true;
      }
      m_clearScreenFromRenderDown = m_input->IsKeyDown(VK_BACK);

      handleRaytracing();
      if (m_shouldClearRaytraceRender) {
        m_debugRenderer->clearRaytraceTexture();
        m_shouldClearRaytraceRender = false;
      }

      m_debugRenderer->frame();
    }
  }
}

void RenderingManager::handleRaytracing() {

  if (m_shouldTriggerRaytraceRender) {
    if (m_settings.computationType == core::ComputationType::FULL_FRAME) {
      // here we schedule the render for next
      // frame, giving the change for the ui to
      // render any extra information pre-render needed
      if (!m_preRaytraceNotificationSent) {
        m_debugRenderer->setPreRaytraceNotification();
        m_preRaytraceNotificationSent = true;
        return;
      }
	  else  {
        core::SceneCamera camera;
        m_debugRenderer->getSceneCamera(&camera);
        m_context->setSceneCamera(&camera);
        m_context->run();
        core::TextureOutput output = m_context->getTextureOutput();
        m_debugRenderer->setRaytraceTexture(&output);
        m_debugRenderer->setPostRaytraceNotification();
        m_shouldTriggerRaytraceRender = false;
        m_preRaytraceNotificationSent= false;
      }

    } else {
      core::SceneCamera camera;
      m_debugRenderer->getSceneCamera(&camera);
      m_context->setSceneCamera(&camera);
      m_context->run();
      core::TextureOutput output = m_context->getTextureOutput();
      m_debugRenderer->setRaytraceTexture(&output);
      m_shouldTriggerRaytraceRender = false;
    }
  }
}

void RenderingManager::loadSceneFromFile(const std::string &path) {

  m_scene = new core::Scene();
  m_scene->loadSceneFromDescription(path);
  m_debugRenderer->initializeDebugScene(m_scene);
  m_context->loadScene(m_scene);
}
} // namespace application
} // namespace mg_ray
