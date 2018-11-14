#pragma once
#include <Windows.h>

#include <memory>

#include "mg_rayLib/foundation/MSWindows/dxWindow.h"
#include "mg_rayLib/rendering/debugRenderer.h"
#include "mg_rayLib/rendering/dxRenderer/implicitSurface.h"
#include "mg_rayLib/rendering/dxRenderer/mesh.h"
#include "mg_rayLib/rendering/dxRenderer/surfaceShader.h"
#include "mg_rayLib/rendering/dxRenderer/blitShader.h"

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
class ImplicitSurface;
class Texture2D;

class Dx11DebugRenderer : public rendering::DebugRenderer {

public:
  Dx11DebugRenderer() = default;
  virtual ~Dx11DebugRenderer() = default;
  bool initialize(foundation::Input *input,
                  core::GlobalSettings *settings) override;
  bool initializeDebugScene(core::Scene *scene) override;
  foundation::Window *getWindow() const override { return m_window; }
  void setRaytraceTexture(core::TextureOutput *texture) override;
  void clearRaytraceTexture() override;
  void getSceneCamera(core::SceneCamera* camera) override;
  // main render loop
  void frame() override;

  LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam,
                                  LPARAM lparam);

private:
  void loadMeshes();
  void render();
  void handleCameraMovement();
  Texture2D *getDx11TextureFromCPUData(core::TextureOutput *texture);

private:
  foundation::DxWindow *m_window = nullptr;
  D3DClass *m_d3dClass;
  ID3D11Device *m_device;
  ID3D11DeviceContext *m_deviceContext;
  Camera3dPivot *m_camera;

  std::unique_ptr<Mesh> sphere;
  std::unique_ptr<Mesh> plane;
  std::unique_ptr<SurfaceShader> m_shader;
  std::unique_ptr<BlitShader> m_blitShader;
  std::vector<ImplicitSurface> m_implicitMeshes;
  Texture2D* m_raytracedTexture = nullptr;
  ID3D11SamplerState* m_linearSampler = nullptr;

  int m_oldMouseX;
  int m_oldMouseY;
};

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
