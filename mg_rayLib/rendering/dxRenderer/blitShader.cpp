
#include "mg_rayLib/rendering/dxRenderer/blitShader.h"
#include "mg_rayLib/rendering/dxRenderer/d3dclass.h"
#undef max
#include "mg_rayLib/core/file_utils.h"
#include "mg_rayLib/rendering/dxRenderer/d3dclass.h"
#include "mg_rayLib/rendering/dxRenderer/shaderUtils.h"
#include <d3d11.h>

namespace mg_ray {
namespace rendering {
namespace dx11 {

BlitShader::~BlitShader() {

  // Release the pixel shader.
  if (m_pixelShader) {
    m_pixelShader->Release();
    m_pixelShader = nullptr;
  }

  // Release the vertex shader.
  if (m_vertexShader) {
    m_vertexShader->Release();
    m_vertexShader = nullptr;
  }
}

bool BlitShader::Initialize(D3DClass *d3dClass, const std::string &path) {

  m_d3dClass = d3dClass;
  // lets read the data from the json

  auto device = m_d3dClass->getDevice();
  // need to patch in the full path of the shader
  auto vsblob = compileVertexShader(
      "shaders/dx11Shaders/blitShaderVS.hlsl", "main", device);
  auto psblob = compilePixelShader(
      "shaders/dx11Shaders/blitShaderPS.hlsl", "main", device);
  m_vertexShader = vsblob.shader;
  m_pixelShader = psblob.shader;

  // cleaning up
  vsblob.releaseBlob();
  psblob.releaseBlob();

  return true;
}

bool BlitShader::render() {

  auto *deviceContext = m_d3dClass->GetDeviceContext();
  // Set the vertex input layout.
  deviceContext->IASetInputLayout(m_layout);

  // Set the vertex and pixel shaders that will be used to render this triangle.
  deviceContext->VSSetShader(m_vertexShader, NULL, 0);
  deviceContext->PSSetShader(m_pixelShader, NULL, 0);

  return true;
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
