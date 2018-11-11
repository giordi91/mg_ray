#pragma once
#include <DirectXMath.h>

namespace mg_ray {
namespace rendering {
namespace dx11 {

class Mesh;
class ImplicitSurface {
public:
  ImplicitSurface() = default;
  inline bool initialize(Mesh *mesh, DirectX::XMMATRIX transform) {
    m_mesh = mesh;
	m_transform = transform;
  };
  bool render();

private:
  DirectX::XMMATRIX m_transform;
  Mesh *m_mesh;
};

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
