#pragma once
#include <d3d11.h>

namespace mg_ray {
namespace rendering {
namespace dx11 {
inline ID3D11Buffer *getVertexBuffer(ID3D11Device *device, int dataSizeInByte,
                                     void *data,
                                     D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
                                     int extraBindFlags = 0,
                                     int miscFlags = 0) {

  D3D11_BUFFER_DESC vertexBufferDesc;
  D3D11_SUBRESOURCE_DATA vertexData;

  // Set up the description of the static vertex buffer.
  vertexBufferDesc.Usage = usage;
  vertexBufferDesc.ByteWidth = dataSizeInByte;
  vertexBufferDesc.BindFlags = (D3D11_BIND_VERTEX_BUFFER | extraBindFlags);
  vertexBufferDesc.CPUAccessFlags =
      usage == D3D11_USAGE_DEFAULT ? 0 : D3D11_CPU_ACCESS_WRITE;
  vertexBufferDesc.MiscFlags = miscFlags;
  vertexBufferDesc.StructureByteStride = 0;

  // Give the subresource structure a pointer to the vertex data.
  vertexData.pSysMem = data;
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  // Now create the vertex buffer.
  ID3D11Buffer *toReturn;
  HRESULT result =
      device->CreateBuffer(&vertexBufferDesc, &vertexData, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
};

inline ID3D11Buffer *
getVertexBufferDynamic(ID3D11Device *device, int dataSizeInByte, void *data,
                       int extraBindFlags = 0, int miscFlags = 0) {

  D3D11_BUFFER_DESC vertexBufferDesc;
  ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
  D3D11_SUBRESOURCE_DATA vertexData;

  // Set up the description of the static vertex buffer.
  vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  vertexBufferDesc.ByteWidth = dataSizeInByte;
  vertexBufferDesc.BindFlags = (D3D11_BIND_VERTEX_BUFFER | extraBindFlags);
  vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  vertexBufferDesc.MiscFlags = miscFlags;
  vertexBufferDesc.StructureByteStride = 0;

  // Give the subresource structure a pointer to the vertex data.
  vertexData.pSysMem = data;
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  // Now create the vertex buffer.
  ID3D11Buffer *toReturn;
  HRESULT result =
      device->CreateBuffer(&vertexBufferDesc, &vertexData, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
};

inline ID3D11Buffer *getIndexBuffer(ID3D11Device *device, int dataSizeInByte,
                                    void *data,
                                    D3D11_USAGE usage = D3D11_USAGE_DEFAULT) {
  D3D11_BUFFER_DESC indexBufferDesc;
  D3D11_SUBRESOURCE_DATA indexData;
  // Set up the description of the static index buffer.
  indexBufferDesc.Usage = usage;
  indexBufferDesc.ByteWidth = dataSizeInByte;
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  indexBufferDesc.CPUAccessFlags = 0;
  indexBufferDesc.MiscFlags = 0;
  indexBufferDesc.StructureByteStride = 0;

  // Give the subresource structure a pointer to the index data.
  indexData.pSysMem = data;
  indexData.SysMemPitch = 0;
  indexData.SysMemSlicePitch = 0;

  // Create the index buffer.
  ID3D11Buffer *toReturn;
  HRESULT result =
      device->CreateBuffer(&indexBufferDesc, &indexData, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
}
inline ID3D11Buffer *getConstantBuffer(ID3D11Device *device,
                                       int dataSizeInByte) {

  D3D11_BUFFER_DESC matrixBufferDesc;
  // Setup the description of the dynamic matrix constant buffer that is in the
  // vertex shader.
  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = dataSizeInByte;
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  // Create the constant buffer pointer so we can access the vertex shader
  // constant buffer from within this class.

  ID3D11Buffer *toReturn;
  HRESULT result = device->CreateBuffer(&matrixBufferDesc, NULL, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
}

inline ID3D11Buffer *getRawBuffer(ID3D11Device *device, int dataSizeInByte,
                                  void *data, int bindFlags = 0) {

  D3D11_SUBRESOURCE_DATA vertexData;

  // Set up the description of the static vertex buffer.
  D3D11_BUFFER_DESC desc;

  ZeroMemory(&desc, sizeof(desc));
  desc.BindFlags =
      D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | bindFlags;
  desc.ByteWidth = dataSizeInByte;
  desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

  vertexData.pSysMem = data;
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  // Now create the vertex buffer.
  ID3D11Buffer *toReturn;
  HRESULT result = device->CreateBuffer(
      &desc, data != nullptr ? &vertexData : nullptr, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
};

inline ID3D11Buffer *getStructuredBuffer(ID3D11Device *device,
                                         int dataSizeInByte, void *data,
                                         int structSize, int bindFlags = 0) {

  D3D11_SUBRESOURCE_DATA vertexData;

  // Set up the description of the static vertex buffer.
  D3D11_BUFFER_DESC desc;

  ZeroMemory(&desc, sizeof(desc));
  desc.BindFlags =
      D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | bindFlags;
  desc.ByteWidth = dataSizeInByte;
  // desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
  desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  desc.StructureByteStride = structSize;

  vertexData.pSysMem = data;
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  // Now create the vertex buffer.
  ID3D11Buffer *toReturn;
  HRESULT result = device->CreateBuffer(
      &desc, data != nullptr ? &vertexData : nullptr, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
};

inline ID3D11Buffer *getStructuredBufferUpdatable(ID3D11Device *device,
                                                  int dataSizeInByte,
                                                  void *data, int structSize,
                                                  int bindFlags = 0) {

  D3D11_SUBRESOURCE_DATA vertexData;

  // Set up the description of the static vertex buffer.
  D3D11_BUFFER_DESC desc;

  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | bindFlags;
  desc.ByteWidth = dataSizeInByte;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  // desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
  desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  desc.StructureByteStride = structSize;

  vertexData.pSysMem = data;
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  // Now create the vertex buffer.
  ID3D11Buffer *toReturn;
  HRESULT result = device->CreateBuffer(
      &desc, data != nullptr ? &vertexData : nullptr, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
};
inline ID3D11Buffer *getRawBufferIndirect(ID3D11Device *device,
                                          int dataSizeInByte, void *data,
                                          int bindFlags = 0) {

  // Set up the description of the static vertex buffer.
  D3D11_SUBRESOURCE_DATA vertexData;
  D3D11_BUFFER_DESC desc;

  ZeroMemory(&desc, sizeof(desc));
  desc.BindFlags =
      D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | bindFlags;
  desc.ByteWidth = dataSizeInByte;
  desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS |
                   D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

  vertexData.pSysMem = data;
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  // Now create the vertex buffer.
  ID3D11Buffer *toReturn;
  HRESULT result = device->CreateBuffer(
      &desc, data != nullptr ? &vertexData : nullptr, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
};

inline ID3D11ShaderResourceView *createBufferSRVRawView(ID3D11Device *device,
                                                        ID3D11Buffer *pBuffer) {
  D3D11_BUFFER_DESC descBuf;
  ZeroMemory(&descBuf, sizeof(descBuf));
  pBuffer->GetDesc(&descBuf);

  D3D11_SHADER_RESOURCE_VIEW_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
  desc.BufferEx.FirstElement = 0;

  // This is a Raw Buffer

  desc.Format = DXGI_FORMAT_R32_TYPELESS;
  desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
  desc.BufferEx.NumElements = descBuf.ByteWidth / 4;

  ID3D11ShaderResourceView *toReturn;
  auto result = device->CreateShaderResourceView(pBuffer, &desc, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
}
inline ID3D11ShaderResourceView *
createBufferSRVStructured(ID3D11Device *device, ID3D11Buffer *pBuffer,
                          int dataSize = 4) {
  D3D11_BUFFER_DESC descBuf;
  ZeroMemory(&descBuf, sizeof(descBuf));
  pBuffer->GetDesc(&descBuf);

  D3D11_SHADER_RESOURCE_VIEW_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  desc.BufferEx.FirstElement = 0;

  // This is a Raw Buffer

  desc.Format = DXGI_FORMAT_UNKNOWN;
  // desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
  desc.BufferEx.NumElements = descBuf.ByteWidth / dataSize;

  ID3D11ShaderResourceView *toReturn;
  auto result = device->CreateShaderResourceView(pBuffer, &desc, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
}

inline ID3D11UnorderedAccessView *createBufferUAVRaw(ID3D11Device *device,
                                                     ID3D11Buffer *buffer) {

  D3D11_BUFFER_DESC descBuf;
  ZeroMemory(&descBuf, sizeof(descBuf));
  buffer->GetDesc(&descBuf);

  D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
  desc.Buffer.FirstElement = 0;

  // This is a Raw Buffer

  desc.Format =
      DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when
                                // creating Raw Unordered Access View
  desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
  desc.Buffer.NumElements = descBuf.ByteWidth / 4;

  ID3D11UnorderedAccessView *toReturn = nullptr;
  HRESULT result = device->CreateUnorderedAccessView(buffer, &desc, &toReturn);
  if (FAILED(result)) {
    toReturn = nullptr;
  }
  return toReturn;
}

template <UINT TNameLength>
inline void setDebugObjectName(_In_ ID3D11DeviceChild *resource,
                               _In_z_ const char (&name)[TNameLength]) {
#if defined(_DEBUG) || defined(PROFILE)
  resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#endif
}
inline void setDebugObjectName(_In_ ID3D11DeviceChild *resource,
                               _In_z_ const char *name) {
#if defined(_DEBUG) || defined(PROFILE)
  resource->SetPrivateData(WKPDID_D3DDebugObjectName,
                           static_cast<unsigned int>(strlen(name) - 1), name);
#endif
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
