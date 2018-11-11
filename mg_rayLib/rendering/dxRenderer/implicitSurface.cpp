#include "mg_rayLib/rendering/dxRenderer/implicitSurface.h"
#include "mg_rayLib/rendering/dxRenderer/buffers_utils.h"
#include "mg_rayLib/rendering/dxRenderer/camera.h"
#include "mg_rayLib/rendering/dxRenderer/mesh.h"

namespace mg_ray {
namespace rendering {
namespace dx11 {
bool ImplicitSurface::initialize(ID3D11Device *device, Mesh *mesh,
                                 DirectX::XMMATRIX transform,
                                 core::SceneMaterial material) {
  m_mesh = mesh;
  m_transform = transform;
  m_material = material;
  matBuffer = getConstantBuffer(device, sizeof(Dx11Material));
  return true;
};

bool ImplicitSurface::render(ID3D11DeviceContext *deviceContext,
                             Camera3dPivot *camera) {

  camera->setCameraMatrixToShader(m_transform);
  // setup the material
  Dx11Material mat;
  mat.ambient.x = 0.1;
  mat.ambient.y = 0.1;
  mat.ambient.z = 0.1;
  DirectX::XMFLOAT3 camPos = camera->getPosition();
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

  mat.diffuse.x = m_material.albedo.x;
  mat.diffuse.y = m_material.albedo.y;
  mat.diffuse.z = m_material.albedo.z;
  mat.specular.w = 0.0f;

  mat.shiness = 100.0f;

  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  ObjectBufferDef *dataPtr;
  unsigned int bufferNumber;

  result = deviceContext->Map(matBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0,
                              &mappedResource);
  if (FAILED(result)) {
    assert(0);
    return false;
  }
  memcpy(mappedResource.pData, &mat, sizeof(Dx11Material));
  deviceContext->Unmap(matBuffer, 0);

  deviceContext->PSSetConstantBuffers(0, 1, &matBuffer);

  m_mesh->render(deviceContext, camera);

  return true;
}
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
