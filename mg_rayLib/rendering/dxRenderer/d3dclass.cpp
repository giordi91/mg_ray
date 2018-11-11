#include "d3dclass.h"
#include <Dxgi1_4.h>
#include <iostream>
//#include <rendering/ImGuiUIdx11.h>
//#include "rendering/RenderingManager.h"
//#include "rendering/PostProcessingManager.h"
namespace mg_ray {
namespace rendering {
namespace dx11 {

IDXGIAdapter3 *findBestAdapter(IDXGIFactory4 *dxgiFactory,
                               D3DVendor givenVendor) {
  // ComPtr<IDXGIAdapter1> curAdapter;
  IDXGIAdapter1 *curAdapter;
  size_t adapterMemory = 0;
  IDXGIAdapter3 *dxgiAdapter = nullptr;
  for (UINT adapterIdx = 0;
       dxgiFactory->EnumAdapters1(adapterIdx, &curAdapter) == S_OK;
       ++adapterIdx) {
    IDXGIAdapter3 *adapter = (IDXGIAdapter3 *)curAdapter;
    if (adapter) {
      DXGI_ADAPTER_DESC2 desc;
      if (SUCCEEDED(adapter->GetDesc2(&desc))) {

        if (givenVendor != D3DVendor::ALL) {
          if (desc.VendorId != (unsigned int)givenVendor) {
            continue;
          }
        }

        if ((desc.VendorId != 0x1414) & (desc.DeviceId != 0x8c)) {
          if (desc.DedicatedVideoMemory > adapterMemory) {
            dxgiAdapter = adapter;
            adapterMemory = desc.DedicatedVideoMemory;
          }
        }
      }
    }
  }
  return dxgiAdapter;
}

bool D3DClass::createDepthStencil(int screenWidth, int screenHeight) {
  // initialize the description of the depth buffer.
  D3D11_TEXTURE2D_DESC depthBufferDesc;
  ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

  // Set up the description of the depth buffer.
  HRESULT result;
  depthBufferDesc.Width = screenWidth;
  depthBufferDesc.Height = screenHeight;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  // Create the texture for the depth buffer using the filled out description.
  result =
      m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
  if (FAILED(result)) {
    return false;
  }

  // initialize the description of the stencil state.
  D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
  ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

  // Set up the description of the stencil state.
  depthStencilDesc.DepthEnable = true;
  depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

  depthStencilDesc.StencilEnable = true;
  depthStencilDesc.StencilReadMask = 0xFF;
  depthStencilDesc.StencilWriteMask = 0xFF;

  // Stencil operations if pixel is front-facing.
  depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // Stencil operations if pixel is back-facing.
  depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // Create the depth stencil state.
  result = m_device->CreateDepthStencilState(&depthStencilDesc,
                                             &m_depthStencilState);
  if (FAILED(result)) {
    return false;
  }

  // Set the depth stencil state.
  m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

  // initialize the depth stencil view.
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
  ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

  // Set up the depth stencil view description.
  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  // Create the depth stencil view.
  result = m_device->CreateDepthStencilView(
      m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
  if (FAILED(result)) {
    return false;
  }
  return true;
}

bool D3DClass::setTransparency() {

  // set transparency
  D3D11_BLEND_DESC blendStateDescription;

  // Clear the blend state description.

  ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
  // Create an alpha enabled blend state description.
  // blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
  blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
  // blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
  blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendStateDescription.RenderTarget[0].RenderTargetWriteMask =
      D3D11_COLOR_WRITE_ENABLE_ALL;

  // Create the blend state using the description.
  ID3D11BlendState *m_alphaEnableBlendingState;
  auto result = m_device->CreateBlendState(&blendStateDescription,
                                           &m_alphaEnableBlendingState);

  float blendFactor[4];
  // Setup the blend factor.
  blendFactor[0] = 0.0f;
  blendFactor[1] = 0.0f;
  blendFactor[2] = 0.0f;
  blendFactor[3] = 0.0f;

  // Turn on the alpha blending.

  m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor,
                                   0xffffffff);
  return true;
}

bool D3DClass::initialize(int screenWidth, int screenHeight, bool vsync,
                          HWND hwnd, bool fullscreen, float screenDepth,
                          float screenNear, D3DVendor vendor) {
  m_vendor = vendor;
  HRESULT result;
  IDXGIFactory4 *factory;
  unsigned int numerator = 0;
  unsigned int denominator = 1;
  D3D_FEATURE_LEVEL featureLevel;
  ID3D11Texture2D *backBufferPtr;
  D3D11_RASTERIZER_DESC rasterDesc;
  D3D11_VIEWPORT viewport;

  // Store the vsync setting.
  m_vsync_enabled = vsync;
  m_hwnd = hwnd;

  // Create a DirectX graphics interface factory.
  result = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void **)&factory);
  if (FAILED(result)) {
    return false;
  }

  DXGI_SWAP_CHAIN_DESC1 swd;
  // ZeroMem(swd);
  ZeroMemory(&swd, sizeof(swd));
  swd.Width = screenWidth;
  swd.Height = screenHeight;
  swd.SampleDesc.Count = 1;
  swd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swd.BufferCount = 2;
  swd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  // Create the swap chain, Direct3D device, and Direct3D device context.
  UINT creationFlags = 0;
  //  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  //#if defined(_DEBUG)
  //  // If the project is in a debug build, enable the debug layer.
  //  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
  //#endif
  DXGI_ADAPTER_DESC2 ad;
  auto dxgiAdapter = findBestAdapter(factory, m_vendor);
  dxgiAdapter->GetDesc2(&ad);
  printf("[VIDEO] Using adapter %ws\n", ad.Description);

  UINT flags = 0;
  // if (debugMode)
#if defined(_DEBUG)
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  D3D_FEATURE_LEVEL requestedLevels[] = {D3D_FEATURE_LEVEL_11_0,
                                         D3D_FEATURE_LEVEL_11_1};

  if (FAILED(D3D11CreateDevice(dxgiAdapter, D3D_DRIVER_TYPE_UNKNOWN, 0, flags,
                               requestedLevels, ARRAYSIZE(requestedLevels),
                               D3D11_SDK_VERSION, &m_device, &featureLevel,
                               &m_deviceContext))) {
    printf("[VIDEO] Failed to create D3D11 device\n");
    return false;
  }

  if (FAILED(factory->CreateSwapChainForHwnd(m_device, hwnd, &swd, nullptr,
                                             nullptr, &m_swapChain))) {
    printf("[VIDEO] Failed to create swap chain\n");
    return false;
  }
  // result = D3D11CreateDeviceAndSwapChain(
  //    NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, &featureLevel, 1,
  //    D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL,
  //    &m_deviceContext);
  // if (FAILED(result)) {
  //  return false;
  //}

  // Get the pointer to the back buffer.
  result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                  (LPVOID *)&backBufferPtr);
  if (FAILED(result)) {
    return false;
  }

  // Create the render target view with the back buffer pointer.
  result = m_device->CreateRenderTargetView(backBufferPtr, NULL,
                                            &m_renderTargetView);

  if (FAILED(result)) {
    return false;
  }

  // Release pointer to the back buffer as we no longer need it.
  backBufferPtr->Release();
  backBufferPtr = 0;

  createDepthStencil(screenWidth, screenHeight);
  setTransparency();
  // Bind the render target view and depth stencil buffer to the output render
  // pipeline.
  m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView,
                                      m_depthStencilView);
  m_currentEnabledBuffer = m_renderTargetView;

  // Setup the raster description which will determine how and what polygons
  // will be drawn.
  rasterDesc.AntialiasedLineEnable = false;
  // rasterDesc.CullMode = D3D11_CULL_BACK;
  rasterDesc.CullMode = D3D11_CULL_NONE;
  rasterDesc.DepthBias = 0;
  rasterDesc.DepthBiasClamp = 0.0f;
  rasterDesc.DepthClipEnable = true;
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.FrontCounterClockwise = false;
  rasterDesc.MultisampleEnable = false;
  rasterDesc.ScissorEnable = false;
  rasterDesc.SlopeScaledDepthBias = 0.0f;

  // Create the rasterizer state from the description we just filled out.
  result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
  if (FAILED(result)) {
    return false;
  }

  rasterDesc.CullMode = D3D11_CULL_NONE;
  result =
      m_device->CreateRasterizerState(&rasterDesc, &m_rasterNoCullingState);

  // Now set the rasterizer state.
  m_deviceContext->RSSetState(m_rasterState);

  // Setup the viewport for rendering.
  viewport.Width = (float)screenWidth;
  viewport.Height = (float)screenHeight;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;

  // Create the viewport.
  m_deviceContext->RSSetViewports(1, &viewport);

  // Setup the projection matrix.
  constexpr float fieldOfView = DirectX::XM_PI / 4.0f;
  float screenAspect = (float)screenWidth / (float)screenHeight;

  // creating selection buffer
  D3D11_TEXTURE2D_DESC textureDesc;
  D3D11_RENDER_TARGET_VIEW_DESC selectTargetViewDesc;

  ///////////////////////// Map's Texture
  // initialize the  texture description.
  ZeroMemory(&textureDesc, sizeof(textureDesc));

  // Setup the texture description.
  // We will have our map be a square
  // We will need to have this texture bound as a render target AND a shader
  // resource
  textureDesc.Width = screenWidth;
  textureDesc.Height = screenHeight;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;

  // Create the texture
  result = m_device->CreateTexture2D(&textureDesc, NULL, &m_selectionBuffer);
  if (FAILED(result)) {
    return false;
  }

  /////////////////////// Map's render Target
  // Setup the description of the render target view.
  selectTargetViewDesc.Format = textureDesc.Format;
  selectTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  // selectTargetViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  selectTargetViewDesc.Texture2D.MipSlice = 0;

  // Create the render target view.
  result = m_device->CreateRenderTargetView(
      m_selectionBuffer, &selectTargetViewDesc, &m_selectionTargetView);
  if (FAILED(result)) {
    return false;
  }

  // staging texture
  ZeroMemory(&textureDesc, sizeof(textureDesc));
  textureDesc.Width = 1;
  textureDesc.Height = 1;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_STAGING;
  textureDesc.BindFlags = 0;
  textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  textureDesc.MiscFlags = 0;
  result = m_device->CreateTexture2D(&textureDesc, NULL, &m_staginTexture);
  if (FAILED(result)) {
    return false;
  }

  // IMGUI
  // Setup ImGui binding

  // ImGui_ImplDX11_Init(hwnd, m_device, m_deviceContext);
  // ImGui::StyleColorsDark();

  return true;
}

void D3DClass::blitToRenderTarget(ID3D11Texture2D *texture) {
  ID3D11Texture2D *pBuffer;
  auto hr =
      m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBuffer);
  m_deviceContext->CopyResource(pBuffer, texture);
}

void D3DClass::cleanup() {
  // Before shutting down set to windowed mode or when you release the swap
  // chain it will throw an exception.
  if (m_swapChain) {
    m_swapChain->SetFullscreenState(false, NULL);
  }

  if (m_rasterState) {
    m_rasterState->Release();
    m_rasterState = nullptr;
  }

  if (m_depthStencilView) {
    m_depthStencilView->Release();
    m_depthStencilView = nullptr;
  }

  if (m_depthStencilState) {
    m_depthStencilState->Release();
    m_depthStencilState = nullptr;
  }

  if (m_depthStencilBuffer) {
    m_depthStencilBuffer->Release();
    m_depthStencilBuffer = nullptr;
  }

  if (m_renderTargetView) {
    m_renderTargetView->Release();
    m_renderTargetView = nullptr;
  }

  if (m_deviceContext) {
    m_deviceContext->Release();
    m_deviceContext = nullptr;
  }

  if (m_device) {
    m_device->Release();
    m_device = nullptr;
  }

  if (m_swapChain) {
    m_swapChain->Release();
    m_swapChain = nullptr;
  }
}

void D3DClass::resize(int width, int height) {
  // releasing references
  m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

  m_renderTargetView->Release();
  // m_selectionTargetView->Release();
  HRESULT hr;
  // Preserve the existing buffer count and format.
  // Automatically choose the width and height to match the client rect for
  // HWNDs.
  hr = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

  // Perform error handling here!

  // Get buffer and create a render-target-view.
  ID3D11Texture2D *pBuffer;
  hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBuffer);
  // Perform error handling here!

  hr = m_device->CreateRenderTargetView(pBuffer, NULL, &m_renderTargetView);
  m_currentEnabledBuffer = m_renderTargetView;
  // Perform error handling here!
  pBuffer->Release();

  m_depthStencilView->Release();
  m_depthStencilBuffer->Release();
  m_depthStencilState->Release();
  createDepthStencil(width, height);

  m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView,
                                      m_depthStencilView);

  setTransparency();
  // m_alphaEnableBlendingState->Release();
  // Set up the viewport.
  D3D11_VIEWPORT vp;
  vp.Width = width;
  vp.Height = height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  m_deviceContext->RSSetViewports(1, &vp);
}

uint32_t D3DClass::getColorAtCoord(int x, int y) {
  std::cout << "reading from " << x << " " << y << std::endl;
  D3D11_BOX srcBox;
  srcBox.left = x;
  srcBox.right = srcBox.left + 1;
  srcBox.top = y;
  srcBox.bottom = srcBox.top + 1;
  srcBox.front = 0;
  srcBox.back = 1;

  m_deviceContext->CopySubresourceRegion(m_staginTexture, 0, 0, 0, 0,
                                         m_selectionBuffer, 0, &srcBox);

  D3D11_MAPPED_SUBRESOURCE msr;
  HRESULT result =
      m_deviceContext->Map(m_staginTexture, 0, D3D11_MAP_READ, 0, &msr);
  if (FAILED(result)) {
    std::cout << "failed to map  sub region";
  }
  BYTE *pixel = (BYTE *)msr.pData;
  std::cout << "color " << int(pixel[0]) << " " << int(pixel[1]) << " "
            << int(pixel[2]) << std::endl;
  uint32_t finalColor = (pixel[0]) | (pixel[1] << 8) | (pixel[2] << 16);
  // copy data
  m_deviceContext->Unmap(m_staginTexture, 0);
  return finalColor;
}

void D3DClass::GetVideoCardInfo(char *cardName, int &memory) {
  strcpy_s(cardName, 128, m_videoCardDescription);
  memory = m_videoCardMemory;
  return;
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
