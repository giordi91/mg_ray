#pragma once
#include <memory>

#ifdef WIN32
#include <Windows.h>
#endif

namespace mg_ray {
namespace core {
struct GlobalSettings;
}
namespace foundation {
class Window;
}
namespace rendering {

class DebugRenderer {

public:
  DebugRenderer() = default;
  bool initialize(core::GlobalSettings *settings);
  inline foundation::Window *getWindow() const { return m_window; }

#ifdef WIN32
  LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam,
                                  LPARAM lparam);
#endif

private:
  // std::unique_ptr<foundation::Window> m_window = nullptr;
  foundation::Window *m_window = nullptr;
  core::GlobalSettings *m_settings;
};

} // namespace rendering
} // namespace mg_ray
