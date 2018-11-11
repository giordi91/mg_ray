#pragma once

namespace mg_ray {
namespace core {

enum class HardwareType { CPU, GPU };

enum class ComputationType { FULL_FRAME, PROGRESSIVE };

struct GlobalSettings {
  int width = 1200;
  int height = 600;
  float farPlane = 1000.0f;
  float nearPlane = 0.01f;
  bool vsync = false;
  bool fullScreen = false;
  HardwareType hardwareType = HardwareType::CPU;
  ComputationType computationType = ComputationType::FULL_FRAME;
  char *name = "Renderer 1.0v";
};
} // namespace core
} // namespace mg_ray
