#include "mg_rayLib/core/dataManipulationHelper.h"
#include "middleware/tiny_obj_loader.h"

namespace mg_ray {
namespace core {
namespace dataIO {

std::unique_ptr<float[]>
fromTinyObjToFlatPointNormalUVBuffer(tinyobj::attrib_t &attr,
                                     tinyobj::shape_t &shape) {

  /*
// Allocating and copying data
size_t sz = attr.vertices_ids.size();
int render_index_size = static_cast<uint32_t>(attr.vertices_ids.size());
int STRIDE;
m_cpu_vtx_data = attr.vertices_ids;

STRIDE = 12;
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
  vtx[curr + 3] = 1.0f;

  vtx[curr + 4] = sourceNorm[n_id * 3];
  vtx[curr + 5] = sourceNorm[n_id * 3 + 1];
  vtx[curr + 6] = sourceNorm[n_id * 3 + 2];
  vtx[curr + 7] = 0.0f;

  vtx[curr + 8] = sourceUv[uv_id * 2];
  vtx[curr + 9] = sourceUv[uv_id * 2 + 1];
  vtx[curr + 10] = 0.0f;
  vtx[curr + 11] = 0.0f;

  idx[i] = i;
}

m_stride = STRIDE;
// creating the buffers
m_vertexBuffer =
    getVertexBuffer(device, sz * STRIDE * sizeof(float), m_vertexs.data());
m_indexBuffer = getIndexBuffer(device, render_index_size * sizeof(int),
                                                           render_index.data());
                                                           */

  return std::unique_ptr<float[]>();
}

} // namespace dataIO
} // namespace core
} // namespace mg_ray
