#pragma once

#include "mg_rayLib/core/renderContext.h"

namespace mg_ray {

namespace foundation {
class Window;
}

namespace core {
class CPURenderContext : public RenderContext {

public:
  CPURenderContext() : RenderContext(){};
  virtual ~CPURenderContext() = default;

  bool initialize() override;
  bool loadScene(char *path) override;
  void run() override;
  void cleanup() override;

private:
  foundation::Window *m_window = nullptr;
};
} // namespace core
} // namespace mg_ray
