#include "bvh.h"
//#include "rendering/debugRenderer.h"
#include "middleware/tbb/include/tbb/parallel_sort.h"
#include <Windows.h>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <intrin.h>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace mg_ray {
namespace core {

const float BuildNodesAABBs::AABB_PADDING = 0.0001f;

// Expands a 10-bit integer into 30 bits
// by inserting 2 zeros after each bit.
inline unsigned int expandBits(unsigned int v) {
  v = (v * 0x00010001u) & 0xFF0000FFu;
  v = (v * 0x00000101u) & 0x0F00F00Fu;
  v = (v * 0x00000011u) & 0xC30C30C3u;
  v = (v * 0x00000005u) & 0x49249249u;
  return v;
}

int getMortonCode(float x, float y, float z) {
  unsigned int ixx = (unsigned int)(x * 1024.0f);
  unsigned int iyy = (unsigned int)(y * 1024.0f);
  unsigned int izz = (unsigned int)(z * 1024.0f);

  unsigned int xx = expandBits(ixx);
  unsigned int yy = expandBits(iyy);
  unsigned int zz = expandBits(izz);
  return xx * 4 + yy * 2 + zz;
}

inline float getMinOf3(float a, float b, float c) {

  float curr = a < b ? a : b;
  float curr2 = curr < c ? curr : c;
  return curr2;
}

inline float getMaxOf3(float a, float b, float c) {
  float curr = a > b ? a : b;
  float curr2 = curr > c ? curr : c;
  return curr2;
}
inline float getMaxOf2(float a, float b) {
  float m = a > b ? a : b;
  return m;
}
inline float getMinOf2(float a, float b) {
  float m = a < b ? a : b;
  return m;
}

inline void buildAABB(const glm::vec3 a, const glm::vec3 b, const glm::vec3 c,
                      std::vector<glm::vec3> &aabbs, int index) {

  float minX = getMinOf3(a.x, b.x, c.x);
  float maxX = getMaxOf3(a.x, b.x, c.x);
  float minY = getMinOf3(a.y, b.y, c.y);
  float maxY = getMaxOf3(a.y, b.y, c.y);
  float minZ = getMinOf3(a.z, b.z, c.z);
  float maxZ = getMaxOf3(a.z, b.z, c.z);

  aabbs[index * 2 + 0] = glm::vec3{minX, minY, minZ};
  aabbs[index * 2 + 1] = glm::vec3{maxX, maxY, maxZ};
}

inline void buildAABB(const std::vector<glm::vec3> &points, glm::vec3 &minP,
                      glm::vec3 &maxP) {

  // avoiding aliasing problem by extracting the pointer from the vector
  const glm::vec3 *pointsPtr = points.data();
  float minX = pointsPtr[0].x;
  float minY = pointsPtr[0].y;
  float minZ = pointsPtr[0].z;

  float maxX = pointsPtr[0].x;
  float maxY = pointsPtr[0].y;
  float maxZ = pointsPtr[0].z;

  int pCount = points.size();
  for (int i = 1; i < pCount; ++i) {
    const auto &p = pointsPtr[i];
    float x = p.x;
    float y = p.y;
    float z = p.z;
    minX = getMinOf2(minX, x);
    maxX = getMaxOf2(maxX, x);

    minY = getMinOf2(minY, y);
    maxY = getMaxOf2(maxY, y);

    minZ = getMinOf2(minZ, z);
    maxZ = getMaxOf2(maxZ, z);
  }

  // TODO(replace with static const
  maxX += ((maxX - minX) < 0.0001f) ? 0.0001f : 0.0f;
  maxY += ((maxY - minY) < 0.0001f) ? 0.0001f : 0.0f;
  maxZ += ((maxZ - minZ) < 0.0001f) ? 0.0001f : 0.0f;

  minP = glm::vec3{minX, minY, minZ};
  maxP = glm::vec3{maxX, maxY, maxZ};
}
void buildCentroids(const std::vector<glm::vec3> &points,
                    const std::vector<unsigned int> &indices,
                    std::vector<glm::vec3> &centroids,
                    std::vector<MortonCode> &mortons,
                    std::vector<glm::vec3> &centroidsNormalized, glm::vec3 minP,
                    glm::vec3 maxP) {

  float deltaX = (maxP.x - minP.x);
  float deltaY = (maxP.y - minP.y);
  float deltaZ = (maxP.z - minP.z);

  deltaX = deltaX < 0.0001 ? 0.0001 : deltaX;
  deltaY = deltaY < 0.0001 ? 0.0001 : deltaY;
  deltaZ = deltaZ < 0.0001 ? 0.0001 : deltaZ;
  int triangleCount = indices.size() / 3;
  float ONE_THIRD = 1.0f / 3.0f;
  for (int i = 0; i < triangleCount; ++i) {
    const glm::vec3 &a = points[indices[i * 3 + 0]];
    const glm::vec3 &b = points[indices[i * 3 + 1]];
    const glm::vec3 &c = points[indices[i * 3 + 2]];

    glm::vec3 centroid =
        glm::vec3{(a.x + b.x + c.x) * ONE_THIRD, (a.y + b.y + c.y) * ONE_THIRD,
                  (a.z + b.z + c.z) * ONE_THIRD};
    centroids[i] = centroid;
    centroidsNormalized[i].x = (centroid.x - minP.x) / deltaX;
    centroidsNormalized[i].y = (centroid.y - minP.y) / deltaY;
    centroidsNormalized[i].z = (centroid.z - minP.z) / deltaZ;

#ifndef NDEBUG
    if (centroidsNormalized[i].x > 1.0f || centroidsNormalized[i].x < 0.0f) {
      assert(0);
    }
    if (centroidsNormalized[i].y > 1.0f || centroidsNormalized[i].y < 0.0f) {
      assert(0);
    }
    if (centroidsNormalized[i].z > 1.0f || centroidsNormalized[i].z < 0.0f) {
      assert(0);
    }
#endif

    int morton =
        getMortonCode(centroidsNormalized[i].x, centroidsNormalized[i].y,
                      centroidsNormalized[i].z);
    mortons[i].code = morton;
    mortons[i].index = i;
  }
}

inline unsigned int customLzcnt(int value) {
#ifdef WIN32
  return __lzcnt(value);
#else
  return __builtin_clz(value);
#endif
}

inline int commonKey(int ki, int kj, int i, int j) {
  int a = (ki == kj) ? i : ki;
  int b = (ki == kj) ? j : kj;
  int value = customLzcnt(a ^ b);
  return ki == kj ? value + 32 : value;
}

inline int getRangeDirection(int index,
                             const std::vector<MortonCode> &mortons) {
  int len = mortons.size();
  int minusId = index == 0 ? index : index - 1;
  int plusId = index == (len - 1) ? index : index + 1;
  int minusCommon =
      commonKey(mortons[minusId].code, mortons[index].code, minusId, index);
  int plusCommon =
      commonKey(mortons[plusId].code, mortons[index].code, plusId, index);
  //@marco-gio : extract this from the 31 bit and flip it
  int tempDir = static_cast<int>((plusCommon - minusCommon) > 0) ? 1 : -1;
  // check for morton range array
  // check min
  tempDir = index == 0 ? 1 : tempDir;
  // check max
  tempDir = index == (len - 1) ? -1 : tempDir;

  return tempDir;
}
int getRangeSplit(int mortonIndex, const std::vector<MortonCode> &mortons,
                  int index, int endRange, int start, int direction) {

  if (mortonIndex == mortons[endRange].code) {
    return (index + endRange) >> 1;
  }

  int mortonsSize = mortons.size();
  int split = 0;
  int nodeCommon =
      commonKey(mortonIndex, mortons[endRange].code, index, endRange);
  for (int t = start >> 1; t >= 1; t >>= 1) {
    int indexToCheck = index + (split + t) * direction;
    if (indexToCheck >= 0 && indexToCheck < mortonsSize) {
      int newCommon = commonKey(mortonIndex, mortons[indexToCheck].code, index,
                                indexToCheck);
      if (newCommon > nodeCommon) {
        // increase the minimum range of the split
        split += t;
      }
    }
  }
  int finalSplit = index + split * direction + min(direction, 0);
#ifndef NDEBUG
  int n = commonKey(mortons[finalSplit].code, mortons[finalSplit + 1].code,
                    index, endRange);
  bool isDup = mortons[finalSplit].code == mortons[finalSplit + 1].code;
  if (!isDup && index != endRange) {
    assert(nodeCommon == n);
  }

#endif
  return finalSplit;
  /*
  int first = index;
  int last = endRange;

  int firstCode = mortons[first].code;
  int lastCode = mortons[last].code;
  int split = first; // initial guess
    int step = last - first;
            int commonPrefix = customLzcnt(firstCode^lastCode);

  do
  {
                  step = (step + 1) >> 1; // exponential decrease
                  int newSplit = split + step; // proposed new position

                  if (newSplit < last)
                  {
                                  unsigned int splitCode =
  mortons[newSplit].code; int splitPrefix = customLzcnt(firstCode ^ splitCode);
                                  if (splitPrefix > commonPrefix)
                                                  split = newSplit; // accept
  proposal
                  }
  } while (step > 1);

  return split;
  */
}

void findSplit(int index, const std::vector<MortonCode> &mortons,
               std::vector<BVHNode> &internalNodes, int mortonsSize) {
  int direction = getRangeDirection(index, mortons);
  int mortonIndex = mortons[index].code;
  // get the sibling id from where we will star searchign
  int initialIndex = (index - direction) < 0 ? 0 : index - direction;
  int idMin =
      commonKey(mortonIndex, mortons[initialIndex].code, index, initialIndex);
  idMin = (index - direction) < 0 ? 0 : idMin;
  int idMax = 2;
  int indexToCheck = index + idMax * direction;
  int check;

  // we check if the index we need to check is inside the range, otherwise we
  // set the check to fail.
  if ((indexToCheck < 0) | (indexToCheck >= mortonsSize)) {
    check = idMin - 1;
    if (indexToCheck < 0) {
      idMax = 2;
    } else {
      // idMax = mortonsSize - 1;
    }
  } else {
    check =
        commonKey(mortonIndex, mortons[indexToCheck].code, index, indexToCheck);
  }

  while (check > idMin) {
    idMax *= 2;
    //@TODO: marco-gio: can we do anything about this?
    int newIndexToCheck = index + idMax * direction;
    if ((newIndexToCheck < mortonsSize) & (newIndexToCheck > 0)) {
      check = commonKey(mortonIndex, mortons[newIndexToCheck].code, index,
                        newIndexToCheck);
    } else {
      break;
    }
  }
  // if(index == 25131)
  //{
  //    int a = index;
  //    int b = index + idMax*direction;
  //    for(int i= min(a,b) ; i<max(a,b) ; ++i )
  //    {
  //  	  std::cout <<i<<"---> "<< commonKey(mortonIndex, mortons[i].code,
  //  index, i)<<" "<<mortonIndex<<" "<<mortons[i].code<< std::endl;
  //
  //    }
  //    int cc = 0;
  //}
  // we found the end of the range, now we binary search the proper end
  //@TODO: marco-gio: optimize this with shifts
  int div = 2;
  int start = 0;
  for (int t = idMax >> 1; t >= 1; t >>= 1) {
    int indexToCheck = index + (start + t) * direction;
    if ((indexToCheck >= 0) & (indexToCheck < mortonsSize)) {
      int secondMorton = mortons[indexToCheck].code;
      int newCommon = commonKey(mortonIndex, secondMorton, index, indexToCheck);
      if (newCommon > idMin) {
        // increase the minimum range of the split
        start += t;
      }
    } else {
      if (indexToCheck <= 0) {
        indexToCheck = 0;
      }
    }
  }
  int endRange = index + start * direction;
  // if (endRange == index) {
  //  int fuuuck = 0;
  //}

  // now we need to find the split position in the range index and end range

  int finalSplit =
      getRangeSplit(mortonIndex, mortons, index, endRange, idMax, direction);

  int i = index;
  int j = endRange;
  BVHNode &node = internalNodes[index];
  if (min(i, j) == finalSplit) {
    node.leftType = LEAF;
  } else {
    node.leftType = INTERNAL;
  }
  node.leftChildIndex = finalSplit;
  if (max(i, j) == finalSplit + 1) {
    node.rightType = LEAF;
  } else {
    node.rightType = INTERNAL;
  }
  node.rightChildIndex = finalSplit + 1;
  node.start = min(i, j);
  node.end = max(j, i);
}

void findSplitRoot(const std::vector<MortonCode> &mortons,
                   std::vector<BVHNode> &internalNodes) {
  int index = 0;
  // int direction = getRangeDirection(index, mortons);
  int direction = getRangeDirection(index, mortons);
  int mortonsSize = mortons.size();
  int mortonIndex = mortons[index].code;
  // get the sibling id from where we will star searching
  int idMin = 0;
  int idMax = 2;
  int indexToCheck = index + idMax * direction;
  int check;

  // we check if the index we need to check is inside the range, otherwise we
  // set the check to fail.
  if (indexToCheck < 0 || indexToCheck >= mortonsSize) {
    check = idMin - 1;
    if (indexToCheck < 0) {
      idMax = 2;
    } else {
      idMax = mortonsSize - 1;
    }
  } else {
    check =
        commonKey(mortonIndex, mortons[indexToCheck].code, index, indexToCheck);
  }

  while (check > idMin) {
    idMax *= 2;
    //@TODO: marco-gio: can we do anything about this?
    int newIndexToCheck = index + idMax * direction;
    if (newIndexToCheck < mortonsSize && newIndexToCheck > 0) {
      check = commonKey(mortonIndex, mortons[newIndexToCheck].code, index,
                        newIndexToCheck);
    } else {
      int x = mortons.size();

      if (newIndexToCheck > x) {
        // idMax = mortons.size();
      } else {
        // idMax = 2;
      }
      break;
    }
  }
  if (index == 54861 || index == 54862) {
    int cazzo = 0;
  }
  int endRange = mortonsSize - 1;
  int finalSplit =
      getRangeSplit(mortonIndex, mortons, index, endRange, idMax, direction);

  int i = index;
  int j = endRange;
  BVHNode &node = internalNodes[index];
  if (min(i, j) == finalSplit) {
    node.leftType = LEAF;
  } else {
    node.leftType = INTERNAL;
  }
  node.leftChildIndex = finalSplit;
  if (max(i, j) == finalSplit + 1) {
    node.rightType = LEAF;
  } else {
    node.rightType = INTERNAL;
  }
  node.rightChildIndex = finalSplit + 1;
  node.start = min(i, j);
  node.end = max(j, i);
}

void buildAABBFromBVHNode(int nodeIndex, const BVHNode &node,
                          const MortonCode *mortons, const glm::vec3 *points,
                          glm::vec3 *treeAABBs) {

  float minX = 99999.0f;
  float minY = 99999.0f;
  float minZ = 99999.0f;

  float maxX = -99999.0f;
  float maxY = -99999.0f;
  float maxZ = -99999.0f;

  for (int i = node.start; i <= node.end; ++i) {
    int triangleIdx = mortons[i].index;
    for (int t = 0; t < 3; ++t) {
      const auto &p = points[triangleIdx * 3 + t];
      minX = getMinOf2(minX, p.x);
      maxX = getMaxOf2(maxX, p.x);

      minY = getMinOf2(minY, p.y);
      maxY = getMaxOf2(maxY, p.y);

      minZ = getMinOf2(minZ, p.z);
      maxZ = getMaxOf2(maxZ, p.z);
    }
  }
  maxX += ((maxX - minX) < 0.0001f) ? 0.0001f : 0.0f;
  maxY += ((maxY - minY) < 0.0001f) ? 0.0001f : 0.0f;
  maxZ += ((maxZ - minZ) < 0.0001f) ? 0.0001f : 0.0f;

  // scale bbox by a tiny bit to add a bit of tollerance

  auto minV = glm::vec3{minX, minY, minZ};
  auto maxV = glm::vec3{maxX, maxY, maxZ};
  auto delta = (maxV - minV) * BuildNodesAABBs::AABB_PADDING;
  treeAABBs[nodeIndex * 2 + 0] = minV - delta;
  treeAABBs[nodeIndex * 2 + 1] = maxV + delta;

  // Debug draw
  // auto debugRender = rendering::DebugRenderer::get_instance();
  // debug->addPersistantDraw(
  //   {mortonLine, triangleCount * 2, rendering::PrimitiveType::LINE});
  // auto minP = glm::vec3{minX, minY, minZ};
  // auto maxP = glm::vec3{maxX, maxY, maxZ};
  // if (debug) {
  //  // nodeIndex == 1956 1957
  //  // if (nodeIndex == 0 || nodeIndex == 3149 || nodeIndex==3150 || nodeIndex
  //  // == 1956 || nodeIndex == 1957)
  //  //{
  //  debugRender->drawAABB(DirectX::XMFLOAT4{minX, minY, minZ, 1.0f},
  //                        DirectX::XMFLOAT4{maxX, maxY, maxZ, 1.0f},
  //                        DirectX::XMFLOAT4{0.0f, 1.0f, 0.0f, 1.0f}, true);
  //  //}
  //}
}
void buildAABBFromBVHNodeCacheFriendly(int nodeIndex, const BVHNode &node,
                                       const MortonCode *mortons,
                                       const glm::vec3 *points,
                                       glm::vec3 *treeAABBs) {

  float minX = 9999.0f;
  float minY = 9999.0f;
  float minZ = 9999.0f;

  float maxX = -9999.0f;
  float maxY = -9999.0f;
  float maxZ = -9999.0f;

  for (int i = node.start; i <= node.end; ++i) {
    // int triangleIdx = mortons[i].index;
    for (int t = 0; t < 3; ++t) {
      const auto &p = points[i + t];
      minX = getMinOf2(minX, p.x);
      maxX = getMaxOf2(maxX, p.x);

      minY = getMinOf2(minY, p.y);
      maxY = getMaxOf2(maxY, p.y);

      minZ = getMinOf2(minZ, p.z);
      maxZ = getMaxOf2(maxZ, p.z);
    }
  }

  treeAABBs[nodeIndex * 2 + 0] = glm::vec3{minX, minY, minZ};
  treeAABBs[nodeIndex * 2 + 1] = glm::vec3{maxX, maxY, maxZ};
}

void buildTreeAABBs(const std::vector<BVHNode> &internalNodes,
                    const std::vector<MortonCode> &mortons,
                    std::vector<glm::vec3> &treeAABBs,
                    const std::vector<unsigned> &indices,
                    const std::vector<glm::vec3> &points)

{
  for (int i = 0; i < mortons.size() - 1; ++i) {
    // HOOK
    buildAABBFromBVHNode(i, internalNodes[i], mortons.data(), points.data(),
                         treeAABBs.data());
  }
}

bool hasDuplicates(std::vector<MortonCode> &mortons) {
  std::unordered_map<unsigned int, bool> data;
  for (auto &m : mortons) {
    auto found = data.find(m.code);
    if (found != data.end()) {
      return true;
    }
    data[m.code] = true;
  }
  return false;
}

void BVH::init(ScenePolygonMesh *mesh) {

  // m_mesh = mesh;
  int triangleCount = mesh->triangleCount/3;
  indices.resize(triangleCount*3);
  points.resize(mesh->triangleCount);
  treeAABBs.resize((triangleCount) * 2);
  mortons.resize(triangleCount);
  internalNodes.resize(triangleCount);

  // we now need to copy the data over
  for (int i = 0; i < triangleCount*3; ++i) {
    glm::vec3 &p = points[i];
    p.x = mesh->triangles[i * 8 + 0];
    p.y = mesh->triangles[i * 8 + 1];
    p.z = mesh->triangles[i * 8 + 2];

    indices[i] = i;
  }

  auto &minP = treeAABBs[0];
  auto &maxP = treeAABBs[1];
  buildAABB(points, minP, maxP);
  BuildCentroids buildCentroids{minP, maxP, &points, &indices, &mortons};
  tbb::parallel_for(tbb::blocked_range<size_t>(0, triangleCount, 100),
                    buildCentroids);

  tbb::parallel_sort(mortons.begin(), mortons.end(),
                     [](MortonCode &first, MortonCode &second) {
                       return first.code < second.code;
                     });

  BuildNodes buildNodes{&mortons, &internalNodes};
  tbb::parallel_for(tbb::blocked_range<size_t>(0, mortons.size() - 1, 100),
                    buildNodes);

  BuildNodesAABBs buildNodesAABBs{mortons.data(), internalNodes.data(),
                                  points.data(), treeAABBs.data()};
  tbb::parallel_for(tbb::blocked_range<size_t>(1, mortons.size() - 1, 100),
                    buildNodesAABBs);
  // buildTreeAABBs(internalNodes, mortons, treeAABBs, indices, points);
}

/*
void BVH::init(MeshDummy *mesh) {

  m_mesh = mesh;
  int triangleCount = mesh->indices().size() / 3;
  const auto &indices = mesh->indices();
  const auto &points = mesh->points();
  treeAABBs.resize(triangleCount * 2);
  mortons.resize(triangleCount);
  internalNodes.resize(triangleCount);

  auto &minP = treeAABBs[0];
  auto &maxP = treeAABBs[1];
  buildAABB(points, minP, maxP);
  // tbb::spin_mutex mutex;
  // BuildRootAABB buildRootAABB{ minP,maxP,points,mutex};
  // tbb::parallel_for(tbb::blocked_range<size_t>(0, points.size(),
  // 1000),buildRootAABB);

  // buildCentroids(points, indices, centroids, mortons, centroidsNormalized,
  // minP,
  //               maxP);
  BuildCentroids buildCentroids{minP, maxP, &points, &indices, &mortons};
  tbb::parallel_for(tbb::blocked_range<size_t>(0, triangleCount, 15000),
                    buildCentroids);

  // std::cout << "has duplicates ? " << hasDuplicates(mortons) << std::endl;

  // sorting the mortons
  // std::sort(mortons.begin(), mortons.end(),
  //          [](MortonCode &first, MortonCode &second) {
  //            return first.code < second.code;
  //          });
  tbb::parallel_sort(mortons.begin(), mortons.end(),
                     [](MortonCode &first, MortonCode &second) {
                       return first.code < second.code;
                     });
  // splatMeshDummy(m_mesh, mortons, m_outMeshDummy, indices);

  // find split
  // findSplitRoot(mortons, internalNodes);
  // for (int i = 0; i < mortons.size() - 1; ++i) {
  //  findSplit(i, mortons, internalNodes, mortons.size());
  //}

  BuildNodes buildNodes{&mortons, &internalNodes};
  tbb::parallel_for(tbb::blocked_range<size_t>(0, mortons.size() - 1, 100),
                    buildNodes);

  BuildNodesAABBs buildNodesAABBs{mortons.data(), internalNodes.data(),
                                  points.data(), treeAABBs.data()};
  tbb::parallel_for(tbb::blocked_range<size_t>(1, mortons.size() - 1, 100),
                    buildNodesAABBs);
  // buildTreeAABBs(internalNodes, mortons, treeAABBs, indices, points);
}
*/

inline float max2(float a, float b) {
  float res = static_cast<float>(a > b);
  return res * a + (1.0f - res) * b;
}

inline float min2(float a, float b) {
  float res = static_cast<float>(a < b);
  return res * a + (1.0f - res) * b;
}
inline bool intersection(glm::vec3 minP, glm::vec3 maxP, glm::vec3 rayOrigin,
                         glm::vec3 invRayDir) {
  float t1 = (minP[0] - rayOrigin[0]) * invRayDir[0];
  float t2 = (maxP[0] - rayOrigin[0]) * invRayDir[0];

  float tmin = min2(t1, t2);
  float tmax = max2(t1, t2);

  for (int i = 1; i < 3; ++i) {
    t1 = (minP[i] - rayOrigin[i]) * invRayDir[i];
    t2 = (maxP[i] - rayOrigin[i]) * invRayDir[i];

    tmax = min(tmax, max(max(t1, t2), tmin));

    // tmin = max(tmin, min(min(t1, t2), tmax));
    float intermediate = min(t1, t2);
    intermediate = min(intermediate, tmax);
    tmin = max(tmin, intermediate);

    // tmax = min(tmax, max(max(t1, t2), tmin));
    intermediate = max(t1, t2);
    intermediate = max(intermediate, tmin);
    tmax = min(intermediate, tmax);
  }

  float intermediate = max(tmin, 0.0f);
  bool result = tmax > intermediate;
  return result;
}

inline bool intersection2(glm::vec3 minP, glm::vec3 maxP, glm::vec3 rayOrigin,
                          glm::vec3 invRayDir) {
  float t1 = (minP[0] - rayOrigin[0]) * invRayDir[0];
  float t2 = (maxP[0] - rayOrigin[0]) * invRayDir[0];

  float tmin = min(t1, t2);
  float tmax = max(t1, t2);

  for (int i = 1; i < 3; ++i) {
    t1 = (minP[i] - rayOrigin[i]) * invRayDir[i];
    t2 = (maxP[i] - rayOrigin[i]) * invRayDir[i];

    tmin = max(tmin, min(min(t1, t2), tmax));
    tmax = min(tmax, max(max(t1, t2), tmin));
  }

  return tmax > max(tmin, 0.0);
}

bool BVH::intersect(const glm::vec3 &rayPosition, const glm::vec3 &rayDir,
                    float &param, unsigned int &outFaceId, float &outU,
                    float &outV, bool bothDirection) const {
  // bool Kdtree::rayCast(glm::vec3 rayPosition, glm::vec3 rayDirection,
  //                     float &param, glm::vec3 &hit, int &faceId,
  //                     std::vector<int> &debugHitAABBs, bool debug) {

  auto rayDirection = glm::normalize(rayDir);
  glm::vec3 invRayDir = {
      1.0f / rayDirection[0],
      1.0f / rayDirection[1],
      1.0f / rayDirection[2],
  };
  bool h = false;
  std::queue<int> toCheck;
  std::vector<int> indicesToIntersect;
  indicesToIntersect.reserve(30);
  toCheck.push(0);
  // std::unordered_set<int> checked;

  while (!toCheck.empty()) {
    int i = toCheck.front();
    // if (checked.find(i) != checked.end()) {
    //  int fuck = 0;
    //} else {
    //  checked.insert(i);
    //}
    // TODO fix copy
    glm::vec3 minP = treeAABBs[i * 2 + 0];
    glm::vec3 maxP = treeAABBs[i * 2 + 1];

    bool intersects = intersection(minP, maxP, rayPosition, invRayDir);
    if (intersects) {
      const BVHNode &node = internalNodes[i];
      if (node.leftType != LEAF) {
        toCheck.push(node.leftChildIndex);
      } else {
        indicesToIntersect.push_back(mortons[node.leftChildIndex].index);
      }
      if (node.rightType != LEAF) {
        toCheck.push(node.rightChildIndex);
      } else {
        indicesToIntersect.push_back(mortons[node.rightChildIndex].index);
      }
    }
    toCheck.pop();
  }

  // const auto &indices = m_mesh->indices();
  // const auto &points = m_mesh->points();
  glm::vec3 outInt;
  float tempParam;
  // TODO if compiler supports change this to maxfloat
  param = 999999.0f;
  // auto debugRender = rendering::DebugRenderer::get_instance();
  const int *indicesCheckPtr = indicesToIntersect.data();
  const unsigned int *indicesPtr = indices.data();
  for (int i = 0; i < indicesToIntersect.size(); ++i) {
    // std::cout << "checking index " << indicesToIntersect[i] << std::endl;

    glm::vec3 vertex0 = points[indicesPtr[indicesCheckPtr[i] * 3 + 0]];
    glm::vec3 vertex1 = points[indicesPtr[indicesCheckPtr[i] * 3 + 1]];
    glm::vec3 vertex2 = points[indicesPtr[indicesCheckPtr[i] * 3 + 2]];

    bool res = RayIntersectsTriangle(rayPosition, rayDirection,
                                     indicesToIntersect[i], outInt, tempParam);
    // debug drawing for the time being
    if ((res == true) & (tempParam < param)) {
      // debugRender->drawPoint(
      //   &outInt.x, DirectX::XMFLOAT4{1.0f, 0.0f, 1.0f, 1.0f}, 3.0f, false);
      // hit = outInt;
      outU = outInt.x;
      outV = outInt.y;
      h = true;
      outFaceId = indicesToIntersect[i];
      param = tempParam;
    }
  }

  // if (h)
  //{
  //    debugRender->drawPoint(
  //  	  &hit.x, DirectX::XMFLOAT4{ 1.0f, 0.0f, 1.0f, 1.0f }, 3.0f, false);
  //}
  return h;
}
bool BVH::RayIntersectsTriangle(const glm::vec3 rayOrigin,
                                const glm::vec3 rayVector, int i,
                                glm::vec3 &outIntersectionPoint,
                                float &param) const {

  // rayVector = rayVector.normalize();
  // const auto &indices = m_mesh->indices();
  // const auto &points = m_mesh->points();
  const float EPSILON = 0.0000001f;
  glm::vec3 vertex0 = points[indices[i * 3 + 0]];
  glm::vec3 vertex1 = points[indices[i * 3 + 1]];
  glm::vec3 vertex2 = points[indices[i * 3 + 2]];
  glm::vec3 edge1, edge2, h, s, q;
  float a, f, u, v;
  edge1 = vertex1 - vertex0;
  edge2 = vertex2 - vertex0;
  h = glm::cross(rayVector, edge2);
  a = glm::dot(edge1, h);
  // TODO remove && for &
  if ((a > -EPSILON) & (a < EPSILON))
    return false;
  f = 1.0f / a;
  s = rayOrigin - vertex0;
  u = f * (glm::dot(s, h));
  // TODO remove || for |
  if (u < 0.0 || u > 1.0)
    return false;
  q = glm::cross(s, edge1);
  v = f * glm::dot(rayVector, q);
  // TODO remove || for |
  if ((v < 0.0) | (u + v > 1.0))
    return false;
  // At this stage we can compute t to find out where the intersection point is
  // on the line.
  float t = f * glm::dot(edge2, q);
  if (t > EPSILON) // ray intersection
  {
    // outIntersectionPoint = rayOrigin + rayVector * t;

    outIntersectionPoint.x = (1.0f - u - v);
    outIntersectionPoint.y = u;
    param = t;
    return true;
  } else // This means that there is a line intersection but not a ray
    // intersection.
    return false;
}

/*
bool AtomsUtils::MeshDummy::intersect(const glm::vec3& orig, const glm::vec3&
dir, float& param, unsigned& outFaceId, float& outU, float& outV, bool
bothDirection) const
{


  dir= dir.normalize();
  const auto &indices = m_mesh->indices();
  const auto &points = m_mesh->points();
  const float EPSILON = 0.0000001;
  glm::vec3 vertex0 = points[indices[i * 3 + 0]];
  glm::vec3 vertex1 = points[indices[i * 3 + 1]];
  glm::vec3 vertex2 = points[indices[i * 3 + 2]];
  glm::vec3 edge1, edge2, h, s, q;
  float a, f, u, v;
  edge1 = vertex1 - vertex0;
  edge2 = vertex2 - vertex0;
  h = rayVector.cross(edge2);
  a = edge1.dot(h);
  if (a > -EPSILON && a < EPSILON)
        return false;
  f = 1 / a;
  s = rayOrigin - vertex0;
  u = f * (s.dot(h));
  if (u < 0.0 || u > 1.0)
        return false;
  q = s.cross(edge1);
  v = f * rayVector.dot(q);
  if (v < 0.0 || u + v > 1.0)
        return false;
  // At this stage we can compute t to find out where the intersection point is
  // on the line.
  float t = f * edge2.dot(q);
  if (t > EPSILON) // ray intersection
  {
        outIntersectionPoint = rayOrigin + rayVector * t;
        return true;
  } else // This means that there is a line intersection but not a ray
                 // intersection.
        return false;
}
*/

void BuildNodes::operator()(const tbb::blocked_range<size_t> &r) const {
  for (int i = r.begin(); i < r.end(); ++i) {
    // if(i == 25130)
    //{
    //	int xx = 0;
    //}
    findSplit(i, *m_mortons, *m_internalNodes, m_mortons->size());
  }
}

void BuildNodesAABBs::operator()(const tbb::blocked_range<size_t> &r) const {

  for (int i = r.begin(); i < r.end(); ++i) {
    buildAABBFromBVHNode(i, m_internalNodes[i], m_mortons, m_points,
                         m_treeAABBs);

    // buildAABBFromBVHNodeCacheFriendly(i, m_internalNodes[i], m_mortons,
    // m_outMeshDummy,
    //                     m_treeAABBs);
  }
}

void BuildRootAABB::operator()(const tbb::blocked_range<size_t> &r) const {

  const glm::vec3 *points = (*m_points).data();
  int start = r.begin();
  float minX = points[start].x;
  float minY = points[start].y;
  float minZ = points[start].z;

  float maxX = points[start].x;
  float maxY = points[start].y;
  float maxZ = points[start].z;
  for (int i = r.begin(); i < r.end(); ++i) {
    const auto &p = points[i];
    minX = getMinOf2(minX, p.x);
    maxX = getMaxOf2(maxX, p.x);

    minY = getMinOf2(minY, p.y);
    maxY = getMaxOf2(maxY, p.y);

    minZ = getMinOf2(minZ, p.z);
    maxZ = getMaxOf2(maxZ, p.z);
  }

  {
    tbb::spin_mutex::scoped_lock(m_mutex);
    *m_minP = glm::vec3{minX, minY, minZ};
    *m_maxP = glm::vec3{maxX, maxY, maxZ};
  }
}

void BuildCentroids::operator()(const tbb::blocked_range<size_t> &r) const {

  const unsigned int *indices = (*m_indices).data();
  const glm::vec3 *points = (*m_points).data();

  double sizeX = m_maxP.x - m_minP.x;
  double sizeY = m_maxP.y - m_minP.y;
  double sizeZ = m_maxP.z - m_minP.z;

  sizeX = sizeX < 0.0001 ? 0.0001 : sizeX;
  sizeY = sizeY < 0.0001 ? 0.0001 : sizeY;
  sizeZ = sizeZ < 0.0001 ? 0.0001 : sizeZ;

  double deltaX = 1.0f / (sizeX);
  double deltaY = 1.0f / (sizeY);
  double deltaZ = 1.0f / (sizeZ);

  // deltaX = deltaX < 0.0001 ? 0.0001 : deltaX;
  // deltaY = deltaY < 0.0001 ? 0.0001 : deltaY;
  // deltaZ = deltaZ < 0.0001 ? 0.0001 : deltaZ;
  // NOTE HERE YOU MIGHT NEED DOUBLES IF YOU HAVE HUGE
  // SCENES
  int triangleCount = (*m_indices).size() / 3;
  float ONE_THIRD = 1.0f / 3.0f;
  for (int i = r.begin(); i < r.end(); ++i) {
    int id1 = indices[i * 3 + 0];
    int id2 = indices[i * 3 + 1];
    int id3 = indices[i * 3 + 2];
    const glm::vec3 &a = points[id1];
    const glm::vec3 &b = points[id2];
    const glm::vec3 &c = points[id3];

    glm::vec3 centroid =
        glm::vec3{(a.x + b.x + c.x) * ONE_THIRD, (a.y + b.y + c.y) * ONE_THIRD,
                  (a.z + b.z + c.z) * ONE_THIRD};
    // centroids[i] = centroid;
    // double cx = (centroid.x - m_minP.x) * deltaX;
    // double cy = (centroid.y - m_minP.y) * deltaY;
    // double cz = (centroid.z - m_minP.z) * deltaZ;

    double cx = (centroid.x - m_minP.x) / sizeX;
    double cy = (centroid.y - m_minP.y) / sizeY;
    double cz = (centroid.z - m_minP.z) / sizeZ;

#ifndef NDEBUG
    if (cx > 1.0 || cx < 0.0) {
      assert(0);
    }
    if (cy > 1.0 || cy < 0.0) {
      assert(0);
    }
    if (cz > 1.0 || cz < 0.0) {
      assert(0);
    }
#endif

    cx = abs(cx);
    cy = abs(cy);
    cz = abs(cz);
    int morton = getMortonCode(cx, cy, cz);
    (*m_mortons)[i].code = morton;
    (*m_mortons)[i].index = i;
  }
}

void BVH::clear() {
  mortons.resize(0);
  internalNodes.resize(0);
  treeAABBs.resize(0);
}
} // namespace core
} // namespace mg_ray
