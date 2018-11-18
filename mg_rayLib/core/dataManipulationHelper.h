#pragma once
#include <memory>

// forward declares
namespace tinyobj {
struct attrib_t;
struct shape_t;
} // namespace tinyobj

namespace mg_ray {
namespace core {
namespace dataIO {

// super simple function that splat a single shape from tiny obj into a
// interleaved contiguous buffer
std::unique_ptr<float[]>
fromTinyObjToFlatPointNormalUVBuffer(tinyobj::attrib_t &attr,
                                     tinyobj::shape_t &shape);

} // namespace dataIO
} // namespace core
} // namespace mg_ray
