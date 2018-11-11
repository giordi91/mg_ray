#include "renderingManager.h"
#ifdef WIN32
#include "mg_rayLib/foundation/MSWindows/input.h"
#include "mg_rayLib/rendering/dxRenderer/dxDebugRenderer.h"
#endif
namespace mg_ray {
namespace application {
bool RenderingManager::initialize() {

  m_input = new foundation::Input();
#ifdef WIN32
  auto* winRenderer  = new rendering::dx11::Dx11DebugRenderer();
  winRenderer->initialize(m_input,&m_settings);
  m_debugRenderer = winRenderer;
#else
  assert(0);
#endif

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
      // Otherwise do the frame processing.
      // if (m_graphics != nullptr) {
      //  result = m_graphics->frame();
      //  if (!result) {
      //    done = true;
      //  }
      //}
      m_debugRenderer->frame();
    }
  }
}
} // namespace application
} // namespace mg_ray
