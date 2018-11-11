#pragma once
#include <d3d11.h>
#include <string>

namespace mg_ray {
namespace rendering {
namespace dx11 {
class D3DClass;
struct LayoutHandle {
  D3D11_INPUT_ELEMENT_DESC *layout;
  int size;
};

class SurfaceShader {
public:
  SurfaceShader() = default;
  ~SurfaceShader();

  bool Initialize(D3DClass *d3dClass, const std::string &path);
  bool render();
  inline ID3D11VertexShader *getVS() const { return m_vertexShader; }
  inline ID3D11PixelShader *getPS() const { return m_pixelShader; }
private :
	LayoutHandle generateMeshLayout();

private:
  ID3D11VertexShader *m_vertexShader = nullptr;
  ID3D11PixelShader *m_pixelShader = nullptr;
  bool isManaged = false;
  D3DClass* m_d3dClass;
  ID3D11InputLayout* m_layout;
};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
