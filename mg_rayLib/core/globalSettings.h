#pragma once

namespace mg_ray {
namespace core {

enum class HardwareType { CPU, GPU };

enum class ComputationType { FULL_FRAME, PROGRESSIVE };

struct GlobalSettings {
  int width = 1200;
  int height = 600;
  HardwareType hardwareType = HardwareType::CPU;
  ComputationType computationType = ComputationType::FULL_FRAME;
  char *name = "Renderer 1.0v";
};
} // namespace core
} // namespace mg_ray
