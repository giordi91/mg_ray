#pragma once
#include <d3d11.h>
#include <string>

namespace mg_ray {
namespace rendering {
namespace dx11 {
class Texture2D {
public:
  Texture2D() = default;
  ~Texture2D() { clear(); }
  // initialize and load data from path
  bool init(ID3D11Device *device, const std::string &path, bool HDR = false,
            bool correctGamma = false);
  bool initFromMemoryRGBAFloat(ID3D11Device *device, float *cpu_data, int width,
                               int height, bool HDR, bool correctGamma);
  // initialize empty texture
  bool initEmpty(ID3D11Device *device, int width, int height, bool HDR,
                 bool correctGamma = false);
  void render(ID3D11DeviceContext *deviceContext, int slot);
  inline ID3D11ShaderResourceView *getSRV() { return m_shaderResourceView; }
  // resizes the texture, keep in mind that no cpu data will be used
  // the new texture will be empty
  void resize(ID3D11Device *device, int width, int height);

private:
  bool createTextureResources(ID3D11Device *device, void *initialData, bool HDR,
                              bool correctGamma);

  bool createTextureResource(ID3D11Device *device, void *initialData,
                             DXGI_FORMAT format);
  void clear();

private:
  int m_width;
  int m_height;
  bool m_isHDR;
  unsigned char *m_cpu_data;
  ID3D11Texture2D *m_texture = nullptr;
  ID3D11ShaderResourceView *m_shaderResourceView = nullptr;
};

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
