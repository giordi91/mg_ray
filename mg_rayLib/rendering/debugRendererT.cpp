#include "debugRendererT.h"
#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/foundation/window.h"

namespace mg_ray {
namespace rendering {
static DebugRenderer *DebugDebugRendererHandle = nullptr;
}
} // namespace mg_ray

#ifdef WIN32
#include "mg_rayLib/foundation/MSWindows/dxWindow.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam,
                         LPARAM lparam) {
  switch (umessage) {
    // Check if the window is being destroyed.
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }

    // Check if the window is being closed.
  case WM_CLOSE: {
    PostQuitMessage(0);
    return 0;
  }

    // All other messages pass to the message handler in the system class.
  default: {
    return mg_ray::rendering::DebugDebugRendererHandle->MessageHandler(
        hwnd, umessage, wparam, lparam);
  }
  }
}
#endif

namespace mg_ray {
namespace rendering {

bool DebugRenderer::initialize(core::GlobalSettings *settings) {

  DebugDebugRendererHandle = this;
  m_settings = settings;

#ifdef WIN32
  foundation::DxWindow::callback = WndProc;
#endif

  m_window = foundation::createWindow(m_settings->width, m_settings->height,
                                      m_settings->name);

  //initialize the swap chain



  return false;
}


/*
inline void BeginScene(float r, float g, float b, float alpha) {
    float color[4];

    // Setup the color to clear the buffer to.
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = alpha;

    // Clear the back buffer.
    m_deviceContext->ClearRenderTargetView(m_currentEnabledBuffer, color);

    // Clear the depth buffer.
    m_deviceContext->ClearDepthStencilView(m_depthStencilView,
                                           D3D11_CLEAR_DEPTH, 1.0f, 0);
  }
*/



void DebugRenderer::frame()
{
}

#ifdef WIN32
LRESULT CALLBACK DebugRenderer::MessageHandler(HWND hwnd, UINT umsg,
                                               WPARAM wparam, LPARAM lparam) {

  // if (m_ui_handler != nullptr) {
  //  bool res = m_ui_handler(hwnd, umsg, wparam, lparam);
  //  if (res) { return true; };
  //}
  // if (ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam)) {
  //  return true;
  //}

  switch (umsg) {
  case WM_SIZE: {
    /*
std::cout << "resizing" << std::endl;
if (m_graphics != nullptr && (m_graphics->m_Direct3D != nullptr) &&
  m_graphics->m_Direct3D->getDevice() != NULL &&
  wparam != SIZE_MINIMIZED) {
ImGui_ImplDX11_InvalidateDeviceObjects();

auto *render = rendering::RenderingManager::get_instance();
render->m_screenWidth = (UINT)LOWORD(lparam);
render->m_screenHeight = (UINT)HIWORD(lparam);
m_graphics->m_Direct3D->resize(render->m_screenWidth,
                               render->m_screenHeight);

auto *deferred = deferred::DeferredTargets::get_instance();
if (render->m_screenWidth != -1 && render->m_screenHeight != 1) {
  deferred->resize(render->m_screenWidth, render->m_screenHeight);
}
// m_graphics->m_Direct3D->m_swapChain->ResizeBuffers(0,
//(UINT)LOWORD(lparam), (UINT)HIWORD(lparam), DXGI_FORMAT_UNKNOWN, 0);

////m_Graphics->m_Direct3D->initialize( (UINT)LOWORD(lparam),
//(UINT)HIWORD(lparam),true,hwnd,false,0.0f,1.0f);
ImGui_ImplDX11_CreateDeviceObjects();
}
  */
    return 0;
  }

    /*
// Check if a key has been pressed on the keyboard.
case WM_KEYDOWN: {
// If a key is pressed send it to the input object so it can record that
// state.
m_input->KeyDown((unsigned int)wparam);
return 0;
}

// Check if a key has been released on the keyboard.
case WM_KEYUP: {
// If a key is released then send it to the input object so it can unset the
// state for that key.
m_input->KeyUp((unsigned int)wparam);
return 0;
}
case WM_LBUTTONDOWN: {
m_input->m_mouse[0] = 1;
m_input->m_mouse[2] = 0;
return 0;
}
case WM_RBUTTONDOWN: {
m_input->m_mouse[1] = 1;
m_input->m_mouse[2] = 0;
return 0;
}
case WM_MBUTTONDOWN: {
m_input->m_mouse[3] = 1;
m_input->m_mouse[2] = 0;
return 0;
}
case WM_MOUSEWHEEL: {
bool forward = (int)GET_WHEEL_DELTA_WPARAM(wparam) > 0;
m_input->m_mouse[2] = forward ? 1 : -1;
return 0;
}
case WM_LBUTTONUP: {
m_input->selection = true;
m_input->m_mouse[0] = 0;
m_input->m_mouse[2] = 0;
return 0;
}
case WM_RBUTTONUP: {
m_input->m_mouse[1] = 0;
m_input->m_mouse[2] = 0;
return 0;
}
case WM_MBUTTONUP: {
m_input->m_mouse[3] = 0;
m_input->m_mouse[2] = 0;
return 0;
}
case WM_MOUSEMOVE: {
m_input->m_mouse_posX = LOWORD(lparam);
m_input->m_mouse_posY = HIWORD(lparam);
}
                     */

  // Any other messages send to the default message handler as our application
  // won't make use of them.
  default: {
    return DefWindowProc(hwnd, umsg, wparam, lparam);
  }
  }
}
#endif

} // namespace rendering
} // namespace mg_ray
