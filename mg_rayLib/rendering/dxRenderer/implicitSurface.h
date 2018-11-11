#pragma once
#include "mg_rayLib/core/scene.h"
#include <DirectXMath.h>
#include <d3d11.h>

namespace mg_ray {
namespace rendering {
namespace dx11 {

class Mesh;
class Camera3dPivot;
class ImplicitSurface {
public:
  ImplicitSurface() = default;
  bool initialize(ID3D11Device *device, Mesh *mesh,
                         DirectX::XMMATRIX transform,
                         core::SceneMaterial material);
  bool render(ID3D11DeviceContext *deviceContext, Camera3dPivot *camera);

private:
  DirectX::XMMATRIX m_transform;
  Mesh *m_mesh;
  core::SceneMaterial m_material;
  ID3D11Buffer *matBuffer;
};

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
