#pragma once
#include <Windows.h>

#include <memory>

#include "mg_rayLib/foundation/MSWindows/dxWindow.h"
#include "mg_rayLib/rendering/debugRenderer.h"
#include "mg_rayLib/rendering/dxRenderer/mesh.h"
#include "mg_rayLib/rendering/dxRenderer/surfaceShader.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
namespace mg_ray {

// forward declares
namespace foundation {
	class Input;
}
namespace core {
struct GlobalSettings;
}

namespace rendering {
namespace dx11 {
class D3DClass;
class Camera3dPivot;
class Mesh;
class SurfaceShader;

class Dx11DebugRenderer : public rendering::DebugRenderer {

public:
  Dx11DebugRenderer() = default;
  virtual ~Dx11DebugRenderer() = default;
  virtual bool initialize(foundation::Input* input,core::GlobalSettings *settings) ;
  inline const foundation::Window *getWindow() const { return m_window; }
  // main render loop
  void frame() override;

  LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam,
                                  LPARAM lparam);

private:
  void loadMeshes();
  void render();
  void handleCameraMovement();

private:
  foundation::DxWindow *m_window = nullptr;
  core::GlobalSettings *m_settings;
  D3DClass *m_d3dClass;
  ID3D11Device *m_device;
  ID3D11DeviceContext *m_deviceContext;
  Camera3dPivot *m_camera;

  std::unique_ptr<Mesh> sphere;
  std::unique_ptr<Mesh> plane;
  std::unique_ptr<SurfaceShader> m_shader;
  foundation::Input* m_input;

  int m_oldMouseX;
  int m_oldMouseY;
};

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
