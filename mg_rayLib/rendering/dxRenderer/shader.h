#pragma once
#include <d3d11.h>

namespace mg_ray {
namespace rendering {
namespace dx11 {


	class Shader
	{
	public:
		Shader() = default;
		virtual ~Shader() = default;

	  inline ID3D11VertexShader *getVS() const { return m_vertexShader; }
	  inline ID3D11PixelShader *getPS() const { return m_pixelShader; }
	  virtual bool render() = 0;
	protected:

	  ID3D11VertexShader *m_vertexShader = nullptr;
	  ID3D11PixelShader *m_pixelShader = nullptr;


	};

}
} // namespace rendering
} // namespace mg_ray
