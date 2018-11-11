#pragma once
#include "mg_rayLib/rendering/debugRenderer.h"
#include <Windows.h>
#include "mg_rayLib/foundation/MSWindows/dxWindow.h"



struct ID3D11Device;
struct ID3D11DeviceContext;
namespace mg_ray {

// forward declares
namespace foundation {
}
namespace core {
struct GlobalSettings;
}

namespace rendering {
namespace dx11 {
class D3DClass;

class Dx11DebugRenderer : public rendering::DebugRenderer {

public:
  Dx11DebugRenderer() = default;
  virtual ~Dx11DebugRenderer() = default;
  virtual bool initialize(core::GlobalSettings *settings) override;
  inline const foundation::Window *getWindow() const { return m_window; }
  // main render loop
  void frame() override;

  LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam,
                                  LPARAM lparam);
private:
  foundation::DxWindow*m_window = nullptr;
  core::GlobalSettings *m_settings;
  D3DClass* m_d3dClass;
  ID3D11Device* m_device;
  ID3D11DeviceContext* m_deviceContext;
};

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
