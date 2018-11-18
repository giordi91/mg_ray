#include "mg_rayLib/rendering/dxRenderer/mesh.h"
#include "mg_rayLib/core/file_utils.h"
#include "mg_rayLib/rendering/dxRenderer/buffers_utils.h"
//#include "rendering/dx11_shaders_layout.h"
#include "mg_rayLib/rendering/dxRenderer/camera.h"
#include "mg_rayLib/rendering/dxRenderer/surfaceShader.h"
#include "middleware/tiny_obj_loader.h"

namespace mg_ray {
namespace rendering {
namespace dx11 {

void Mesh::load(ID3D11Device *device, tinyobj::attrib_t &attr,
                tinyobj::shape_t &shape, SurfaceShader *shader) {
  m_shader = shader;
  // Allocating and copying data
  size_t sz = attr.vertices_ids.size();
  render_index_size = static_cast<uint32_t>(attr.vertices_ids.size());
  int STRIDE;
  m_cpu_vtx_data = attr.vertices_ids;

  STRIDE = 8;
  m_stride = STRIDE;
  m_vertexs.resize(sz * STRIDE);
  // allocate memory for index buffer
  render_index.resize(render_index_size);
  const float *const sourceVtx = attr.vertices.data();
  const float *const sourceNorm = attr.normals.data();
  const float *const sourceUv = attr.texcoords.data();

  float *const vtx = m_vertexs.data();
  unsigned int *const idx = render_index.data();
  //#pragma omp parallel for
  for (int i = 0; i < sz; ++i) {
    const auto vtx_id = attr.vertices_ids[i];
    const auto uv_id = attr.text_coords_ids[i];
    const auto n_id = attr.normals_ids[i];
    const auto curr = i * STRIDE;

    vtx[curr + 0] = sourceVtx[vtx_id * 3];
    vtx[curr + 1] = sourceVtx[vtx_id * 3 + 1];
    vtx[curr + 2] = sourceVtx[vtx_id * 3 + 2];

    vtx[curr + 3] = sourceNorm[n_id * 3];
    vtx[curr + 4] = sourceNorm[n_id * 3 + 1];
    vtx[curr + 5] = sourceNorm[n_id * 3 + 2];

    vtx[curr + 6] = sourceUv[uv_id * 2];
    vtx[curr + 7] = sourceUv[uv_id * 2 + 1];

    idx[i] = i;
  }

  m_stride = STRIDE;

  // creating the buffers
  m_vertexBuffer =
      getVertexBuffer(device, sz * STRIDE * sizeof(float), m_vertexs.data());
  m_indexBuffer = getIndexBuffer(device, render_index_size * sizeof(int),
                                 render_index.data());
}

void Mesh::loadFromFile(ID3D11Device *device, const std::string &path,
                        SurfaceShader *shader) {
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  tinyobj::attrib_t data;
  std::string err;
  tinyobj::LoadObj(&data, &shapes, &materials, &err, path.c_str(), 0, true);
  load(device, data, shapes[0], shader);
}
void Mesh::render(ID3D11DeviceContext *deviceContext, Camera3dPivot *camera) {

  m_shader->render();

  // Set vertex buffer stride and offset.
  unsigned int stride[1] = {sizeof(int) * m_stride};
  unsigned int offset[1] = {0};
  // Set the vertex buffer to active in the input assembler so it can be
  // rendered.
  deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, stride, offset);

  // Set the index buffer to active in the input assembler so it can be
  // rendered.
  deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

  // Set the type of primitive that should be rendered from this vertex buffer,
  // in this case triangles.
  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  deviceContext->DrawIndexed(render_index_size, 0, 0);
  // setting const buffer
}

void Mesh::translate(float x, float y, float z) {
  DirectX::XMFLOAT4X4 meshTView;
  XMStoreFloat4x4(&meshTView, transform);
  meshTView._41 = x;
  meshTView._42 = y;
  meshTView._43 = z;
  transform = XMLoadFloat4x4(&meshTView);
}
} // namespace dx11
} // namespace rendering

} // namespace mg_ray
