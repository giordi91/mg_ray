#pragma once

namespace mg_ray {
namespace core {

class RenderContext {

public:
  virtual ~RenderContext()=default;
  virtual bool initialize() = 0;
  virtual bool loadScene(char *) = 0;
  virtual void run() = 0;
  virtual void cleanup() = 0;
};

} // namespace core
} // namespace mg_ray
