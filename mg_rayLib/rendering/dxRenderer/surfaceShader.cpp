
#include "mg_rayLib/rendering/dxRenderer/surfaceShader.h"
#include "mg_rayLib/rendering/dxRenderer/d3dclass.h"
#undef max
#include "mg_rayLib/core/file_utils.h"
#include "mg_rayLib/rendering/dxRenderer/d3dclass.h"
#include "mg_rayLib/rendering/dxRenderer/shaderUtils.h"
#include <d3d11.h>

namespace mg_ray {
namespace rendering {
namespace dx11 {

SurfaceShader::~SurfaceShader() {

  // if (m_layout) {
  //  m_layout->Release();
  //  m_layout = nullptr;
  //}

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

inline void generatePositionFloat4(D3D11_INPUT_ELEMENT_DESC *ptr) {
  ptr->SemanticName = "POSITION";
  ptr->SemanticIndex = 0;
  ptr->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  ptr->InputSlot = 0;
  ptr->AlignedByteOffset = 0;
  ptr->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  ptr->InstanceDataStepRate = 0;
}

inline void generateNormalsFloat4(D3D11_INPUT_ELEMENT_DESC *ptr,
                                  unsigned int offset = 16) {
  ptr->SemanticName = "NORMAL";
  ptr->SemanticIndex = 0;
  ptr->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  ptr->InputSlot = 0;
  ptr->AlignedByteOffset = offset;
  ptr->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  ptr->InstanceDataStepRate = 0;
}

inline void generateUVsFloat4(D3D11_INPUT_ELEMENT_DESC *ptr,
                              unsigned int offset = 32) {
  ptr->SemanticName = "TEXCOORD";
  ptr->SemanticIndex = 0;
  ptr->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  ptr->InputSlot = 0;
  ptr->AlignedByteOffset = offset;
  ptr->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  ptr->InstanceDataStepRate = 0;
}

inline void zeroOutLayouts(LayoutHandle &handle) {
  for (int i = 0; i < handle.size; ++i) {
    ZeroMemory(handle.layout + i, sizeof(D3D11_INPUT_ELEMENT_DESC));
  }
}

LayoutHandle SurfaceShader::generateMeshLayout() {
  // basic mesh layout
  LayoutHandle basicMeshHandle{new D3D11_INPUT_ELEMENT_DESC[3], 3};
  zeroOutLayouts(basicMeshHandle);
  generatePositionFloat4(basicMeshHandle.layout);
  generateNormalsFloat4(basicMeshHandle.layout + 1, 16);
  generateUVsFloat4(basicMeshHandle.layout + 2, 32);
  return basicMeshHandle;
}
ID3D11InputLayout *getLayout(ID3D11Device *device, LayoutHandle handle,
                             ID3D10Blob *shaderBlob) {

  ID3D11InputLayout *out = 0;
  auto *ptr = handle.layout;
  int size = handle.size;
  auto *buffptr = shaderBlob->GetBufferPointer();
  auto buffsize = shaderBlob->GetBufferSize();
  HRESULT result =
      device->CreateInputLayout(ptr, size, buffptr, buffsize, &out);
  if (FAILED(result)) {
    printf("Failed GetRequestProperty [%x]\n", result);
    std::cout << "error in getting layout" << std::endl;
  }
  return out;
}

bool SurfaceShader::Initialize(D3DClass *d3dClass, const std::string &path) {

  m_d3dClass = d3dClass;
  // lets read the data from the json

  auto device = m_d3dClass->getDevice();
  // need to patch in the full path of the shader
  auto vsblob = compileVertexShader(
      "shaders/dx11Shaders/debugForwardMeshVS.hlsl", "main", device);
  auto psblob = compilePixelShader(
      "shaders/dx11Shaders/debugForwardMeshPS.hlsl", "main", device);
  m_vertexShader = vsblob.shader;
  m_pixelShader = psblob.shader;

  LayoutHandle meshLayout = generateMeshLayout();
  // lets grab the vertex layout if we processed the shader
  m_layout = getLayout(device, meshLayout, vsblob.shaderBlob);

  // cleaning up
  vsblob.releaseBlob();
  psblob.releaseBlob();

  return true;
}

bool SurfaceShader::render() {

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
