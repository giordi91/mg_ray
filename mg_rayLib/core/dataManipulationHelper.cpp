#include "mg_rayLib/core/dataManipulationHelper.h"
#include "middleware/tiny_obj_loader.h"

namespace mg_ray {
namespace core {
namespace dataIO {

std::unique_ptr<float[]>
fromTinyObjToFlatPointNormalUVBuffer(const tinyobj::attrib_t &attr,
                                     const tinyobj::shape_t &shape,
                                     unsigned int &vertexCount) {

  vertexCount = static_cast<unsigned int>(attr.vertices_ids.size());
  int render_index_size = static_cast<uint32_t>(attr.vertices_ids.size());

  const int STRIDE = 8;
  const float *const sourceVtx = attr.vertices.data();
  const float *const sourceNorm = attr.normals.data();
  const float *const sourceUv = attr.texcoords.data();

  auto vtx = std::unique_ptr<float[]>(new float[vertexCount * STRIDE]);
#pragma omp parallel for
  for (int i = 0; i < vertexCount; ++i) {
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
  }

  return vtx;
}

} // namespace dataIO
} // namespace core
} // namespace mg_ray
