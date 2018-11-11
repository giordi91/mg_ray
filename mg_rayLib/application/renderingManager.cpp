#include "renderingManager.h"
#ifdef WIN32
#include "mg_rayLib/rendering/dxRenderer/dxDebugRenderer.h"
#endif
namespace mg_ray {
namespace application {
bool RenderingManager::initialize() {

 
  m_debugRenderer = new rendering::dx11::Dx11DebugRenderer();
  m_debugRenderer->initialize(&m_settings);

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
  bool result = true;

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
      //if (m_input->IsKeyDown(VK_ESCAPE)) {
      //  done = true;
      //  continue;
      //}
      // Otherwise do the frame processing.
      //if (m_graphics != nullptr) {
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
