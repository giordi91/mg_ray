#pragma once
#pragma comment(lib, "dxgi.lib")  // used to extract info from the hardware
#pragma comment(lib, "d3d11.lib") // setting up the context etc
#pragma comment(lib, "d3dcompiler.lib") // compiling shaders
#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "d3dx10.lib")

#include "mg_rayLib/rendering/dxRenderer/renderTarget.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>

enum class D3DVendor { NVIDIA = 0x10DE, AMD = 0x1002, Intel = 0x8086, ALL };

namespace mg_ray {
namespace rendering {
namespace dx11 {
class D3DClass {

  D3DClass(const D3DClass &) = delete;
  D3DClass &operator=(D3DClass const &) = delete; // private assignment operator

public:
  D3DClass() = default;
  ~D3DClass() = default;
  bool createDepthStencil(int screenWidth, int screenHeight);
  bool setTransparency();
  bool initialize(int, int, bool, HWND, bool, float, float, D3DVendor vendor);
  void cleanup();
  void blitToRenderTarget(ID3D11Texture2D *texture);
  void setViewport(int screenW, int screenH) {
    D3D11_VIEWPORT viewport;
    // Setup the viewport for rendering.
    viewport.Width = (float)screenW;
    viewport.Height = (float)screenH;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // Create the viewport.
    m_deviceContext->RSSetViewports(1, &viewport);
  };

  inline void disableCulling() {
    m_deviceContext->RSSetState(m_rasterNoCullingState);
  }
  inline void enableCulling() { m_deviceContext->RSSetState(m_rasterState); }
  inline void beginScene(float r, float g, float b, float alpha) {
    float color[4];

    // Setup the color to clear the buffer to.
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = alpha;

    // Clear the back buffer.
    m_deviceContext->ClearRenderTargetView(m_currentEnabledBuffer, color);

    // Clear the depth buffer.
    m_deviceContext->ClearDepthStencilView(m_depthStencilView,
                                           D3D11_CLEAR_DEPTH, 1.0f, 0);
  }
  inline void endScene() {
    // Present the back buffer to the screen since rendering is complete.
    if (m_vsync_enabled) {
      // Lock to screen refresh rate.
      m_swapChain->Present(1, 0);
    } else {
      // Present as fast as possible.
      m_swapChain->Present(0, 0);
    }
  }

  inline ID3D11Device *getDevice() { return m_device; }
  inline ID3D11DeviceContext *GetDeviceContext() { return m_deviceContext; }
  inline ID3D11RenderTargetView *getRenderTarget() {
    return m_currentEnabledBuffer;
  }
  inline void setRenderTarget(ID3D11RenderTargetView *target,
                              ID3D11DepthStencilView *depth) {
    m_deviceContext->OMSetRenderTargets(1, &target, depth);
  }
  inline void enableColorBuffer(bool useDepth) {
    if (useDepth) {
      m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView,
                                          m_depthStencilView);

      m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
    } else {
      m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
    }
    m_currentEnabledBuffer = m_renderTargetView;
  }

  inline void bindShadowMapOnly(ID3D11DepthStencilView *depthView,
                                bool clear = true) {

    if (clear) {
      m_deviceContext->ClearDepthStencilView(
          depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
    }
    ID3D11RenderTargetView *rtview = nullptr;
    m_deviceContext->OMSetRenderTargets(1, &rtview, depthView);
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
  }

  inline void
  enableColorBufferWithExternalDepth(ID3D11DepthStencilView *depthView) {
    m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthView);

    m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
    m_currentEnabledBuffer = m_renderTargetView;
  }

  inline void enableSelectionBuffer() {
    m_deviceContext->OMSetRenderTargets(1, &m_selectionTargetView,
                                        m_depthStencilView);
    m_currentEnabledBuffer = m_selectionTargetView;
  }
  void resize(int width, int height);

  uint32_t getColorAtCoord(int x, int y);

  void GetVideoCardInfo(char *, int &);

  IDXGISwapChain1 *m_swapChain = nullptr;

  HWND m_hwnd;

private:
  bool m_vsync_enabled;
  int m_videoCardMemory;
  char m_videoCardDescription[128];
  ID3D11Device *m_device = nullptr;
  ID3D11DeviceContext *m_deviceContext = nullptr;
  ID3D11Texture2D *m_depthStencilBuffer = nullptr;
  ID3D11DepthStencilState *m_depthStencilState = nullptr;
  ID3D11RasterizerState *m_rasterState = nullptr;
  ID3D11RasterizerState *m_rasterNoCullingState = nullptr;

  ID3D11Texture2D *m_selectionBuffer = nullptr;
  ID3D11Texture2D *m_staginTexture = nullptr;
  ID3D11RenderTargetView *m_renderTargetView = nullptr;
  ID3D11RenderTargetView *m_selectionTargetView = nullptr;
  ID3D11DepthStencilView *m_depthStencilView = nullptr;
  ID3D11RenderTargetView *m_currentEnabledBuffer;

  D3DVendor m_vendor;
};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
