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

class SurfaceShader;
class Camera3dPivot;
class Mesh {
public:
  Mesh() { transform = DirectX::XMMatrixIdentity(); }
  void load(ID3D11Device *device, tinyobj::attrib_t &attr,
            tinyobj::shape_t &shape, SurfaceShader* shader);
  void loadFromFile(ID3D11Device *device, const std::string &path,
                    SurfaceShader* shader);
  void render(ID3D11DeviceContext *context , Camera3dPivot* camera);
  void translate(float x, float y, float z);

public:
  std::vector<float> m_vertexs;
  std::vector<unsigned int> render_index;
  std::vector<float> m_tangents;
  unsigned int render_index_size;
  ID3D11Buffer *m_vertexBuffer;
  ID3D11Buffer *m_indexBuffer;
  DirectX::XMMATRIX transform;
  int m_stride;
  std::vector<unsigned int> m_cpu_vtx_data;
  SurfaceShader* m_shader;
  ID3D11Buffer* matBuffer;
};
} // namespace dx11
} // namespace rendering
} // namespace mg_ray
