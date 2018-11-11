#pragma once
#include "mg_rayLib/foundation/window.h"

namespace mg_ray {
namespace foundation {
class Input;
}
namespace core {
struct GlobalSettings;
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
  virtual bool initialize(foundation::Input* input,core::GlobalSettings *settings) = 0;
  virtual foundation::Window *getWindow() const = 0;
  // main render loop
  virtual void frame() = 0;

protected:
  foundation::Window *m_window = nullptr;
  core::GlobalSettings *m_settings;
  foundation::Input* m_input;
};

} // namespace rendering
} // namespace mg_ray
