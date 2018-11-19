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
  //rendering settings
  int SPP = 10;
  int maxRecursion = 10;
  float aperture = 0.01f;
  float focusDistance = 10.0f;
  float vFov = 45.0f;
  float tMin = 0.001f;
  float tMax = 1000.0f;
};
} // namespace core
} // namespace mg_ray
