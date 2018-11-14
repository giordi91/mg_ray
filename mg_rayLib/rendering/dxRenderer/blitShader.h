
#pragma once
#include "mg_rayLib/rendering/dxRenderer/shader.h"
#include <d3d11.h>
#include <string>

namespace mg_ray {
namespace rendering {
namespace dx11 {
class D3DClass;

class BlitShader : Shader {
public:
  BlitShader() : Shader(){};
  virtual ~BlitShader();

  bool Initialize(D3DClass *d3dClass, const std::string &path);
  bool render() override;

private:
  ID3D11VertexShader *m_vertexShader = nullptr;
  ID3D11PixelShader *m_pixelShader = nullptr;
  D3DClass *m_d3dClass;
  ID3D11InputLayout *m_layout =nullptr;
};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
