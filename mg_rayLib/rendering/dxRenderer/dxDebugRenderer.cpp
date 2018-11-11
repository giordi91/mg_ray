#include "dxDebugRenderer.h"
#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/foundation/MSWindows/dxWindow.h"
#include "mg_rayLib/rendering/dxRenderer/d3dclass.h"
#include <d3d11.h>

namespace mg_ray {

namespace rendering {
namespace dx11 {
static Dx11DebugRenderer *DebugDebugRendererHandle = nullptr;

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
    return DebugDebugRendererHandle->MessageHandler(hwnd, umessage, wparam,
                                                    lparam);
  }
  }
}

bool Dx11DebugRenderer::initialize(core::GlobalSettings *settings) {

  DebugDebugRendererHandle = this;
  m_settings = settings;
  foundation::DxWindow::callback = WndProc;

  m_window = new foundation::DxWindow();
  m_window->initialize(m_settings->width, m_settings->height, m_settings->name);
  m_d3dClass = new D3DClass();

  m_d3dClass->initialize(m_settings->width, m_settings->height, 0 /*no v-sync*/,
                         m_window->getHWND(), 0 /*no full screen*/,
                         m_settings->nearPlane, m_settings->farPlane,
                         D3DVendor::ALL);
  m_deviceContext = m_d3dClass->GetDeviceContext();
  m_device = m_d3dClass->getDevice();
  return true;
}

void Dx11DebugRenderer::frame() {

  m_d3dClass->beginScene(0.5f, 0.5f, 0.5f, 1.0f);
  m_d3dClass->endScene();
}

// windows crap manageent
LRESULT CALLBACK Dx11DebugRenderer::MessageHandler(HWND hwnd, UINT umsg,
                                                   WPARAM wparam,
                                                   LPARAM lparam) {

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

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
