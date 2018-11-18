#include "dxDebugRenderer.h"

#include <d3d11.h>

#include "mg_rayLib/core/globalSettings.h"
#include "mg_rayLib/core/renderContext.h"
#include "mg_rayLib/core/scene.h"
#include "mg_rayLib/foundation/MSWindows/dxWindow.h"
#include "mg_rayLib/foundation/input.h"
#include "mg_rayLib/rendering/dxRenderer/buffers_utils.h"
#include "mg_rayLib/rendering/dxRenderer/camera.h"
#include "mg_rayLib/rendering/dxRenderer/d3dclass.h"
#include "mg_rayLib/rendering/dxRenderer/implicitSurface.h"
#include "mg_rayLib/rendering/dxRenderer/texture2D.h"
#include <iostream>

namespace mg_ray {

namespace rendering {
namespace dx11 {
static Dx11DebugRenderer *DebugDebugRendererHandle = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam,
                         LPARAM lparam) {
  switch (umessage) {
    // Check if the window is being destroyed.
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }

    // Check if the window is being closed.
  case WM_CLOSE: {
    PostQuitMessage(0);
    return 0;
  }

    // All other messages pass to the message handler in the system class.
  default: {
    return DebugDebugRendererHandle->MessageHandler(hwnd, umessage, wparam,
                                                    lparam);
  }
  }
}

ID3D11SamplerState *createLinearSampler(ID3D11Device *device) {
  ID3D11SamplerState *samplerState;
  // Create the two samplers
  D3D11_SAMPLER_DESC samDesc;
  ZeroMemory(&samDesc, sizeof(samDesc));
  samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samDesc.AddressU = samDesc.AddressV = samDesc.AddressW =
      D3D11_TEXTURE_ADDRESS_WRAP;
  samDesc.MaxAnisotropy = 1;
  samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samDesc.MaxLOD = D3D11_FLOAT32_MAX;
  device->CreateSamplerState(&samDesc, &samplerState);
  return samplerState;
}

bool Dx11DebugRenderer::initialize(foundation::Input *input,
                                   core::GlobalSettings *settings) {

  DebugDebugRendererHandle = this;
  m_settings = settings;
  foundation::DxWindow::callback = WndProc;
  m_input = input;

  m_window = new foundation::DxWindow();
  m_window->initialize(m_settings->width, m_settings->height, m_settings->name);
  m_d3dClass = new D3DClass();

  m_d3dClass->initialize(m_settings->width, m_settings->height,
                         m_settings->vsync, m_window->getHWND(),
                         D3DVendor::ALL);
  m_deviceContext = m_d3dClass->GetDeviceContext();
  m_device = m_d3dClass->getDevice();

  // create the camera
  m_camera = new Camera3dPivot(m_d3dClass, static_cast<float>(settings->width),
                               static_cast<float>(settings->height),
                               settings->nearPlane, settings->farPlane);
  m_camera->setLookAt(0.0f, 0.0f, 0.0f);
  m_camera->setPosition(0.0f, 0.0f, -10.0f);

  // load shaders
  m_shader = std::make_unique<SurfaceShader>();
  m_shader->Initialize(m_d3dClass, "");

  m_blitShader = std::make_unique<BlitShader>();
  m_blitShader->Initialize(m_d3dClass, "");

  m_linearSampler = createLinearSampler(m_device);

  // lets load the needed implicit geometries
  loadMeshes();

  // initialize constant buffer used to draw polygonal meshes
  m_matBuffer =
      getConstantBuffer(m_d3dClass->getDevice(), sizeof(Dx11Material));
  return true;
}

void Dx11DebugRenderer::loadImplicitScene(core::Scene *scene) {
  int count = static_cast<int>(scene->m_implicitMeshes.size());
  for (int i = 0; i < count; ++i) {
    const core::SceneImplicitMesh &sceneM = scene->m_implicitMeshes[i];
    if (sceneM.type == core::IMPLICIT_MESH_TYPE::SPHERE) {
      // if the type is a sphere our life is simpler,
      // we just build a  uniform scale matrix to reflect the radius
      // and translate the sphere in position
      auto translate = DirectX::XMMatrixTranslation(
          sceneM.data1.x, sceneM.data1.y, sceneM.data1.z);
      auto scale = DirectX::XMMatrixScaling(sceneM.data1.w, sceneM.data1.w,
                                            sceneM.data1.w);
      auto transform = DirectX::XMMatrixMultiply(scale, translate);
      ImplicitSurface surf;
      surf.initialize(m_d3dClass->getDevice(), sphere.get(), transform,
                      sceneM.material);
      m_implicitMeshes.push_back(surf);
    } else {

      // in the case of a plane our life is a bit more complicated
      DirectX::XMFLOAT4 up{0.0f, 1.0f, 0.0f, 0.0f};
      DirectX::XMFLOAT4 normal{sceneM.data1.x, sceneM.data1.y, sceneM.data1.z,
                               0.0f};

      DirectX::XMVECTOR upv =
          DirectX::XMVector3Normalize(DirectX::XMLoadFloat4(&up));
      DirectX::XMVECTOR normalv =
          DirectX::XMVector3Normalize(DirectX::XMLoadFloat4(&normal));

      // lets perform a a big scale to approximate an infinite plane
      auto transform = DirectX::XMMatrixScaling(1000.0f, 1000.0f, 1000.0f);

      auto dot = DirectX::XMVector3Dot(upv, normalv);
      // check whether or not we need to rotate, the main idea is, you assume no
      // rotation is
      // perfectly vertical, you extract the angle from the dot with the up
      // vector and plane normal generate a rotation on the perpendicular vector
      // and the angle. this will perform the rotation.
      if ((dot.m128_f32[0] - 1.0f) > 0.001f) {
        auto cross =
            DirectX::XMVector4Normalize(DirectX::XMVector3Cross(upv, normalv));
        auto anglev = DirectX::XMVector4AngleBetweenVectors(upv, normalv);
        float angle = anglev.m128_f32[0];

        auto rotation = DirectX::XMMatrixRotationAxis(cross, angle);
        auto translate = DirectX::XMMatrixTranslation(
            normal.x * sceneM.data1.w, normal.y * sceneM.data1.w,
            normal.z * sceneM.data1.w);
        transform = DirectX::XMMatrixMultiply(rotation, translate);
      }

      ImplicitSurface surf;
      surf.initialize(m_d3dClass->getDevice(), plane.get(), transform,
                      sceneM.material);
      m_implicitMeshes.push_back(surf);
    }
  }
  m_sceneMode = SceneMode::IMPLICIT;
}

void Dx11DebugRenderer::loadTrianglesScene(core::Scene *scene) {

  int count = static_cast<int>(scene->m_polygonMeshes.size());
  m_polygonMeshes.reserve(count);
  for (int i = 0; i < count; ++i) {
    // lets instantiate a mesh per scene mesh
    Mesh mesh;
    const core::ScenePolygonMesh &sceneMesh = scene->m_polygonMeshes[i];
    mesh.initFromFlatBufferPosNormalUV8(
        m_d3dClass->getDevice(), sceneMesh.triangles.get(),
        sceneMesh.triangleCount, m_shader.get());
    m_polygonMeshes.emplace_back(DebugMesh{mesh, sceneMesh.material});
  }
  m_sceneMode = SceneMode::POLYGONS;
}
bool Dx11DebugRenderer::initializeDebugScene(core::Scene *scene) {

  int count = static_cast<int>(scene->m_implicitMeshes.size());
  // assuming implicit meshes, right now does not allow mixed implicit and
  // triangles
  if (count != 0) {
    loadImplicitScene(scene);
  } else {
    // otherwise we load polygons
    loadTrianglesScene(scene);
  }
  return true;
}
void Dx11DebugRenderer::setRaytraceTexture(core::TextureOutput *texture) {
  switch (texture->type) {
  case (core::TextureOutputType::CPU): {
    if (m_raytracedTexture != nullptr) {
      delete m_raytracedTexture;
      m_raytracedTexture = nullptr;
    }
    m_raytracedTexture = getDx11TextureFromCPUData(texture);
    break;
  }
  case (core::TextureOutputType::CUDA): {
    assert(0 && "Not implemented yet");
    break;
  }
  case (core::TextureOutputType::DX11): {
    assert(0 && "Not implemented yet");
    break;
  }
  }
}
void Dx11DebugRenderer::clearRaytraceTexture() {
  if (m_raytracedTexture != nullptr) {
    delete m_raytracedTexture;
    m_raytracedTexture = nullptr;
  }
}
void Dx11DebugRenderer::getSceneCamera(core::SceneCamera *camera) {
  camera->vFov = 45.0f;
  camera->aperture = 0.1f;
  camera->focusDistance = 10;
  DirectX::XMMATRIX view =
      m_camera->getViewInverse(DirectX::XMMatrixIdentity());
  // copying the camera to the right place we need, aint pretty but it s faster
  // than jump around between different datatypes, we just have 16 contiguos
  // floats
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4 *)(&camera->view[0].x), view);
}

void Dx11DebugRenderer::frame() {

  m_d3dClass->beginScene(0.5f, 0.5f, 0.5f, 1.0f);
  render();
  m_d3dClass->endScene();
}
void Dx11DebugRenderer::render() {
  handleCameraMovement();
  m_camera->Render();

  if (m_raytracedTexture != nullptr) {
    // lets render the texture instead of the debug scene
    m_blitShader->render();
    m_raytracedTexture->render(m_deviceContext, 0);
    m_deviceContext->PSSetSamplers(0, 1, &m_linearSampler);

    m_deviceContext->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    m_deviceContext->Draw(4, 0);
    return;
  }

  if (m_sceneMode == SceneMode::IMPLICIT) {
    // draw debug geometries to make sure everything works
    for (int i = 0; i < m_implicitMeshes.size(); ++i) {
      m_implicitMeshes[i].render(m_d3dClass->GetDeviceContext(), m_camera);
    }
  } else {
    for (int i = 0; i < m_polygonMeshes.size(); ++i) {
      m_camera->setCameraMatrixToShader(DirectX::XMMatrixIdentity());
      // setup the material
      Dx11Material mat;
      mat.ambient.x = 0.1;
      mat.ambient.y = 0.1;
      mat.ambient.z = 0.1;
      DirectX::XMFLOAT3 camPos = m_camera->getPosition();
      ;
      mat.cameraPosition.x = camPos.x;
      mat.cameraPosition.y = camPos.y;
      mat.cameraPosition.z = camPos.z;
      mat.cameraPosition.w = 1.0f;

      mat.lightPosition.x = 10.0f;
      mat.lightPosition.y = 10.0f;
      mat.lightPosition.z = 10.0f;
      mat.lightPosition.w = 1.0f;

      mat.specular.x = 1.0f;
      mat.specular.y = 1.0f;
      mat.specular.z = 1.0f;
      mat.specular.w = 0.0f;

      const core::SceneMaterial &material = m_polygonMeshes[i].material;
      mat.diffuse.x = material.albedo.x;
      mat.diffuse.y = material.albedo.y;
      mat.diffuse.z = material.albedo.z;
      mat.specular.w = 0.0f;

      mat.shiness = 100.0f;

      HRESULT result;
      D3D11_MAPPED_SUBRESOURCE mappedResource;
      ObjectBufferDef *dataPtr;
      unsigned int bufferNumber;

      ID3D11DeviceContext *deviceContext = m_d3dClass->GetDeviceContext();
      result = m_d3dClass->GetDeviceContext()->Map(
          m_matBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
      assert(SUCCEEDED(result));
      memcpy(mappedResource.pData, &mat, sizeof(Dx11Material));
      deviceContext->Unmap(m_matBuffer, 0);

      deviceContext->PSSetConstantBuffers(0, 1, &m_matBuffer);

      m_polygonMeshes[i].mesh.render(deviceContext, m_camera);
    }
  }
}
void Dx11DebugRenderer::loadMeshes() {
  sphere = std::make_unique<Mesh>();
  sphere->loadFromFile(m_device, "sphere.obj", m_shader.get());
  plane = std::make_unique<Mesh>();
  plane->loadFromFile(m_device, "plane.obj", m_shader.get());
}
void Dx11DebugRenderer::handleCameraMovement() {
  // if (ImGui::GetIO().WantCaptureMouse) {
  //  return;
  //}

  float deltaX = float(m_oldMouseX - m_input->m_mouse_posX);
  float deltaY = float(m_oldMouseY - m_input->m_mouse_posY);
  if (m_input->m_mouse[0] == 1) {
    // m_camera->panCamera(deltaX, deltaY);
    m_camera->rotCamera(deltaX, deltaY);
  } else if (m_input->m_mouse[3] == 1) {
    m_camera->panCamera(deltaX, deltaY);
  } else if (m_input->m_mouse[1] == 1) {
    m_camera->zoomCamera(deltaX, deltaY);
  }

  // storing old position
  m_oldMouseX = m_input->m_mouse_posX;
  m_oldMouseY = m_input->m_mouse_posY;
}

Texture2D *
Dx11DebugRenderer::getDx11TextureFromCPUData(core::TextureOutput *texture) {

  auto *tex2D = new Texture2D();
  tex2D->initFromMemoryRGBAFloat(m_device, static_cast<float *>(texture->data),
                                 texture->width, texture->height, true, false);
  return tex2D;
}

// windows crap management
LRESULT CALLBACK Dx11DebugRenderer::MessageHandler(HWND hwnd, UINT umsg,
                                                   WPARAM wparam,
                                                   LPARAM lparam) {

  // if (m_ui_handler != nullptr) {
  //  bool res = m_ui_handler(hwnd, umsg, wparam, lparam);
  //  if (res) { return true; };
  //}
  // if (ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam)) {
  //  return true;
  //}

  switch (umsg) {
  case WM_SIZE: {
    /*
std::cout << "resizing" << std::endl;
if (m_graphics != nullptr && (m_graphics->m_Direct3D != nullptr) &&
  m_graphics->m_Direct3D->getDevice() != NULL &&
  wparam != SIZE_MINIMIZED) {
ImGui_ImplDX11_InvalidateDeviceObjects();

auto *render = rendering::RenderingManager::get_instance();
render->m_screenWidth = (UINT)LOWORD(lparam);
render->m_screenHeight = (UINT)HIWORD(lparam);
m_graphics->m_Direct3D->resize(render->m_screenWidth,
                               render->m_screenHeight);

auto *deferred = deferred::DeferredTargets::get_instance();
if (render->m_screenWidth != -1 && render->m_screenHeight != 1) {
  deferred->resize(render->m_screenWidth, render->m_screenHeight);
}
// m_graphics->m_Direct3D->m_swapChain->ResizeBuffers(0,
//(UINT)LOWORD(lparam), (UINT)HIWORD(lparam), DXGI_FORMAT_UNKNOWN, 0);

////m_Graphics->m_Direct3D->initialize( (UINT)LOWORD(lparam),
//(UINT)HIWORD(lparam),true,hwnd,false,0.0f,1.0f);
ImGui_ImplDX11_CreateDeviceObjects();
}
  */
    return 0;
  }

  // Check if a key has been pressed on the keyboard.
  case WM_KEYDOWN: {
    // If a key is pressed send it to the input object so it can record that
    // state.
    m_input->KeyDown((unsigned int)wparam);
    return 0;
  }

  // Check if a key has been released on the keyboard.
  case WM_KEYUP: {
    // If a key is released then send it to the input object so it can unset the
    // state for that key.
    m_input->KeyUp((unsigned int)wparam);
    return 0;
  }
  case WM_LBUTTONDOWN: {
    m_input->m_mouse[0] = 1;
    m_input->m_mouse[2] = 0;
    return 0;
  }
  case WM_RBUTTONDOWN: {
    m_input->m_mouse[1] = 1;
    m_input->m_mouse[2] = 0;
    return 0;
  }
  case WM_MBUTTONDOWN: {
    m_input->m_mouse[3] = 1;
    m_input->m_mouse[2] = 0;
    return 0;
  }
  case WM_MOUSEWHEEL: {
    bool forward = (int)GET_WHEEL_DELTA_WPARAM(wparam) > 0;
    m_input->m_mouse[2] = forward ? 1 : -1;
    return 0;
  }
  case WM_LBUTTONUP: {
    m_input->selection = true;
    m_input->m_mouse[0] = 0;
    m_input->m_mouse[2] = 0;
    return 0;
  }
  case WM_RBUTTONUP: {
    m_input->m_mouse[1] = 0;
    m_input->m_mouse[2] = 0;
    return 0;
  }
  case WM_MBUTTONUP: {
    m_input->m_mouse[3] = 0;
    m_input->m_mouse[2] = 0;
    return 0;
  }
  case WM_MOUSEMOVE: {
    m_input->m_mouse_posX = LOWORD(lparam);
    m_input->m_mouse_posY = HIWORD(lparam);
  }

  // Any other messages send to the default message handler as our application
  // won't make use of them.
  default: {
    return DefWindowProc(hwnd, umsg, wparam, lparam);
  }
  }
}

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
