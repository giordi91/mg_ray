#include "texture2D.h"
#include "mg_rayLib/core/file_utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "middleware/stb_image.h"

namespace mg_ray {
namespace rendering {
namespace dx11 {

inline unsigned char *getTextureDataFromFile(const std::string &path,
                                             int *outWidth, int *outHeight) {
  unsigned char *cpu_data =
      stbi_load(path.c_str(), outWidth, outHeight, 0, STBI_rgb_alpha);
  if (cpu_data == nullptr) {
    std::cout << "[Error] left texture load returned nullptr: " << path
              << std::endl;
    return nullptr;
  }
  return cpu_data;
}

bool Texture2D::init(ID3D11Device *device, const std::string &path, bool HDR,
                     bool correctGamma) {

  m_cpu_data = getTextureDataFromFile(path, &m_width, &m_height);

  assert(m_cpu_data != nullptr);
  if (m_cpu_data == nullptr) {
    return false;
  }
  bool res = createTextureResources(device, m_cpu_data, HDR, correctGamma);
  assert(res == true);
  return res;
}

bool Texture2D::initFromMemoryRGBAFloat(ID3D11Device *device, float *cpu_data,
                                        int width, int height, bool HDR,
                                        bool correctGamma) {
  assert(cpu_data != nullptr);
  if (cpu_data == nullptr) {
    return false;
  }
  m_width = width;
  m_height = height;
  m_isHDR = HDR;
  DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  return createTextureResource(device, cpu_data, format);
}

bool Texture2D::initEmpty(ID3D11Device *device, int width, int height, bool HDR,
                          bool correctGamma) {
  m_width = width;
  m_height = height;
  bool res = createTextureResources(device, nullptr, HDR, correctGamma);
  assert(res == true);
  return res;
}

void Texture2D::render(ID3D11DeviceContext *deviceContext, int slot) {
  deviceContext->PSSetShaderResources(slot, 1, &m_shaderResourceView);
}

void Texture2D::resize(ID3D11Device *device, int width, int height) {
  clear();
  initEmpty(device, width, height, m_isHDR);
}

bool Texture2D::createTextureResource(ID3D11Device *device, void *initialData,
                                      DXGI_FORMAT format) {
  D3D11_TEXTURE2D_DESC texDesc;
  texDesc.Width = m_width;
  texDesc.Height = m_height;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = format;
  texDesc.CPUAccessFlags = 0;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags = 0;
  texDesc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA *dataPtr = nullptr;
  D3D11_SUBRESOURCE_DATA initData;
  if (initialData != nullptr) {

    initData.pSysMem = initialData;
    int stride =  m_isHDR ? 16 : 4;
    initData.SysMemPitch = static_cast<UINT>(m_width * stride);
    initData.SysMemSlicePitch = static_cast<UINT>(0);
    dataPtr = &initData;
  }

  // Create the Texture Resource
  HRESULT hr = device->CreateTexture2D(&texDesc, dataPtr, &m_texture);
  if (hr != S_OK) {
    return false;
  }

  // The Shader Resource view description
  D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
  SMViewDesc.Format = texDesc.Format;
  SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
  SMViewDesc.TextureCube.MostDetailedMip = 0;
  // create the resource view
  hr = device->CreateShaderResourceView(m_texture, &SMViewDesc,
                                        &m_shaderResourceView);
  if (hr != S_OK) {
    return false;
  }
  return true;
}

bool Texture2D::createTextureResources(ID3D11Device *device, void *initialData,
                                       bool HDR, bool correctGamma) {
  m_isHDR = HDR;
  // Description of each face

  DXGI_FORMAT format;
  if (HDR) {
    if (correctGamma) {
      format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    } else {
      format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    }
  } else {
    if (correctGamma) {
      format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    } else {
      format = DXGI_FORMAT_R8G8B8A8_UNORM;
    }
  }
  return createTextureResource(device, initialData, format);
}

void Texture2D::clear() {
  if (m_cpu_data) {
    delete[] m_cpu_data;
  }
  if (m_texture) {
    m_texture->Release();
  }
  if (m_shaderResourceView) {
    m_shaderResourceView->Release();
  }
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
