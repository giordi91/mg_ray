#pragma once
#include "DirectXMath.h"
#include <d3d11.h>
#include <vector>

namespace tinyobj {
struct attrib_t;
struct shape_t;
} // namespace tinyobj

namespace mg_ray {
namespace rendering {
namespace dx11 {

struct Dx11Material {
  DirectX::XMFLOAT4 cameraPosition;
  DirectX::XMFLOAT4 lightPosition;
  DirectX::XMFLOAT4 ambient;
  DirectX::XMFLOAT4 specular;
  DirectX::XMFLOAT4 diffuse;
  float shiness;
  float padding1;
  float padding2;
  float padding3;
};

class SurfaceShader;
class Camera3dPivot;
class Mesh {
public:
  Mesh() { transform = DirectX::XMMatrixIdentity(); }
  void load(ID3D11Device *device, tinyobj::attrib_t &attr,
            tinyobj::shape_t &shape, SurfaceShader *shader);
  void loadFromFile(ID3D11Device *device, const std::string &path,
                    SurfaceShader *shader);
  void initFromFlatBufferPosNormalUV8(ID3D11Device *device, const float *data,
                                      unsigned int vertexCount,
                                      SurfaceShader *shader);
  void render(ID3D11DeviceContext *context, Camera3dPivot *camera);
  void translate(float x, float y, float z);

public:
  unsigned int m_vertexCount;
  ID3D11Buffer *m_vertexBuffer;
  ID3D11Buffer *m_indexBuffer;
  DirectX::XMMATRIX transform;
  int m_stride;
  SurfaceShader *m_shader;
};

} // namespace dx11
} // namespace rendering
} // namespace mg_ray
