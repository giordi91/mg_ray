#include "mg_rayLib/foundation/MSwindows/dxWindow.h"
#include <cassert>

namespace mg_ray {
namespace foundation {

WNDPROC DxWindow::callback = nullptr;
bool DxWindow::initialize(int width, int height, const char *name) {
  WNDCLASSEX wc{};

  m_width = width;
  m_height = height;
  m_name = name;

  assert(callback != nullptr && "No callback was provided for the MSWindow window");
  // Get the instance of this application.
  m_hinstance = GetModuleHandle(NULL);

  // Give the application a name.
  std::wstring stemp = std::wstring(m_name.begin(), m_name.end());
  LPCWSTR sw = stemp.c_str();

  // Setup the windows class with default settings.
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = callback;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = m_hinstance;
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hIconSm = wc.hIcon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = sw;
  wc.cbSize = sizeof(WNDCLASSEX);

  // Register the window class.
  RegisterClassEx(&wc);

  // Create the window with the screen settings and get the handle to it.
  constexpr DWORD style =
      WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  RECT wr{0, 0, (LONG)width, (LONG)height};
  // needed to create the window of the right size, or wont match the gui
  AdjustWindowRectEx(&wr, style, false, NULL);
  m_hwnd =
      CreateWindowEx(0, sw, sw, style, 0, 0, wr.right - wr.left,
                     wr.bottom - wr.top, NULL, NULL, GetModuleHandle(NULL), 0);

  if (m_hwnd == nullptr) {
    return false;
  }
  // Bring the window up on the screen and set it as main focus.
  ShowWindow(m_hwnd, SW_SHOWDEFAULT);
  UpdateWindow(m_hwnd);
  SetForegroundWindow(m_hwnd);
  SetFocus(m_hwnd);

  // Hide the mouse cursor.
  ShowCursor(true);
  return true;
}
} // namespace foundation
} // namespace mg_ray
