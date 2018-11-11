#include "mg_rayLib/rendering/dxRenderer/renderTarget.h"
#include "mg_rayLib/rendering/dxRenderer/d3dClass.h"
#include <cassert>

namespace mg_ray {
class D3DClass;
namespace rendering {
namespace dx11 {
bool RenderTarget::init(D3DClass *d3dClass, int width, int height,
                        bool hasDepth, bool hdr) {

  m_d3dClass = d3dClass;
  return createRenderTarget(d3dClass->getDevice(), width, height, hasDepth,
                            hdr);
}
void RenderTarget::bindAsTexture(int slot, bool useDepth) {}
void RenderTarget::bindAsRT() {
  m_d3dClass->setRenderTarget(m_renderingTargetView, m_dsv);
}
void RenderTarget::bindAsRTWithExternalDepth(ID3D11DeviceContext *deviceContext,
                                             ID3D11DepthStencilView *depth) {
  m_d3dClass->setRenderTarget(m_renderingTargetView, depth);
}
void RenderTarget::resize(ID3D11Device *device, int width, int height) {
  clearData();
  createRenderTarget(device, width, height, m_hasDepth, m_isHDR);
}
bool RenderTarget::createRenderTarget(ID3D11Device *device, int width,
                                      int height, bool hasDepth, bool hdr) {
  DXGI_FORMAT format =
      hdr ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;

  m_width = width;
  m_height = height;
  m_isHDR = hdr;
  m_hasDepth = hasDepth;
  // Description of each face
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
  texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags = 0;
  texDesc.MiscFlags = 0;

  // Create the Texture Resource
  HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_texture);
  assert(hr == S_OK);
  if (hr != S_OK) {
    return false;
  }

  // The Shader Resource view description
  D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
  SMViewDesc.Format = texDesc.Format;
  SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
  SMViewDesc.TextureCube.MostDetailedMip = 0;
  // If we have created the texture resource for the six faces
  // we create the Shader Resource View to use in our shaders.
  hr = device->CreateShaderResourceView(m_texture, &SMViewDesc,
                                        &m_shaderResourceView);
  assert(hr == S_OK);
  if (hr != S_OK) {
    return false;
  }

  /////////////////////// Map's render Target
  // Setup the description of the render target view.
  D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
  renderTargetViewDesc.Format = texDesc.Format;
  renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  renderTargetViewDesc.Texture2D.MipSlice = 0;

  // Create the render target view.
  hr = device->CreateRenderTargetView(m_texture, &renderTargetViewDesc,
                                      &m_renderingTargetView);
  assert(hr == S_OK);
  if (FAILED(hr)) {
    return false;
  }

  if (m_hasDepth) {
    // doDepth
    assert(0);
  }
  return true;
}
void RenderTarget::clearData() {
  if (m_dsv) {
    m_dsv->Release();
    m_dsv = nullptr;
  }
  if (m_renderingTargetView) {
    m_renderingTargetView->Release();
    m_renderingTargetView = nullptr;
  }
  if (m_shaderResourceView) {
    m_shaderResourceView->Release();
    m_shaderResourceView = nullptr;
  }
  if (m_texture) {
    m_texture->Release();
    m_texture = nullptr;
  }
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
