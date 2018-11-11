#pragma once

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
  virtual ~DebugRenderer() {
    if (m_window) {
      delete m_window;
    }
  }
  virtual bool initialize(core::GlobalSettings *settings) = 0;
  inline foundation::Window *getWindow() const { return m_window; }
  // main render loop
  virtual void frame() = 0;

private:
  foundation::Window *m_window = nullptr;
  core::GlobalSettings *m_settings;

};

} // namespace rendering
} // namespace mg_ray
