#pragma once

#include <d3d11.h>
#include <string>

namespace mg_ray {
namespace rendering {
namespace dx11 {

class D3DClass;
class RenderTarget {
public:
  RenderTarget() = default;
  bool init(D3DClass *device, int width, int height, bool hasDepth, bool hdr);
  void bindAsTexture( int slot,
                     bool useDepth);
  void bindAsRT();
  void bindAsRTWithExternalDepth(ID3D11DeviceContext *deviceContext,
                                 ID3D11DepthStencilView *depth);
  ID3D11RenderTargetView **getRTView() { return &m_renderingTargetView; }
  ID3D11ShaderResourceView **getShaderView() { return &m_shaderResourceView; }
  void resize(ID3D11Device *device, int width, int height);

private:
  bool createRenderTarget(ID3D11Device *device, int width, int height,
                          bool hasDepth, bool hdr);
  void clearData();

private:
  D3DClass *m_d3dClass = nullptr;
  ID3D11Texture2D *m_texture = nullptr;
  ID3D11ShaderResourceView *m_shaderResourceView = nullptr;
  ID3D11RenderTargetView *m_renderingTargetView = nullptr;
  ID3D11DepthStencilView *m_dsv = nullptr;

  int m_width;
  int m_height;
  bool m_isHDR;
  bool m_hasDepth;
};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
