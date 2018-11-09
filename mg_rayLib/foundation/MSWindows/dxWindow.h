
#pragma once
#include "mg_rayLib/foundation/window.h"
#include <string>
#include <windows.h>

#define WIN32_LEAN_AND_MEAN

namespace mg_ray {
namespace foundation {
class D3D12Application;

class DxWindow : public Window {

public:
  DxWindow()=default;
  virtual ~DxWindow() = default;
  bool initialize(int screenWidth, int screenHeight, const char *name);
  inline HWND getHWND() const { return m_hwnd; }
  inline HINSTANCE getHInstance() const { return m_hinstance; }
  //static callback that needs to be set before the window is created 
  static WNDPROC callback;

private:
  // deleting assignment operator and copy constructors
  DxWindow(const DxWindow &) = delete;
  DxWindow &operator=(const DxWindow &) = delete;

private:
  std::string m_name;
  HINSTANCE m_hinstance;
  HWND m_hwnd;
  int m_width;
  int m_height;
};
} // namespace foundation
} // namespace mg_ray
