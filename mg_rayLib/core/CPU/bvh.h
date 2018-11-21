#pragma once

//#include <ImathBox.h>
//#include <ImathVec.h>
#include "middleware/glm/glm.hpp"

#include "mg_rayLib/core/scene.h"
#include "middleware/tbb/include/tbb/blocked_range.h"
#include "middleware/tbb/include/tbb/spin_mutex.h"
#include <vector>

namespace mg_ray {
namespace core {
class MeshDummy {

public:
  std::vector<glm::vec3> &points() { return m_points; };
  std::vector<unsigned int> &indices() { return m_indices; };

  // private:
  std::vector<glm::vec3> m_points;
  std::vector<unsigned int> m_indices;
  bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, float &param,
                 unsigned int &outFaceId, float &outU, float &outV,
                 bool bothDirection) const;
};

enum BVHNodeType { INTERNAL, LEAF };
struct BVHNode {

  int leftChildIndex;
  int rightChildIndex;
  int start;
  int end;
  BVHNodeType leftType;
  BVHNodeType rightType;
};

struct MortonCode {
  unsigned int code;
  int index;
};
class BVH {

public:
  BVH() = default;

  // void init(MeshDummy *mesh);
  void init(ScenePolygonMesh *mesh);
  void initMulti(ScenePolygonMesh *mesh, int count);
  void clear();

  bool intersect(const glm::vec3 &orig, const glm::vec3 &dir, float &param,
                 unsigned int &outFaceId, float &outU, float &outV,
                 bool bothDirection) const;
  bool RayIntersectsTriangle(const glm::vec3 rayOrigin,
                             const glm::vec3 rayVector, int i,
                             glm::vec3 &outIntersectionPoint,
                             float &param) const;
  inline int getMeshIndex(int triangleIndex, int &localTriangleIndex) {
    for (int i = 0; i < m_verticesScan.size() - 1; ++i) {
      if (triangleIndex < m_verticesScan[i + 1]) {
        localTriangleIndex = triangleIndex - m_verticesScan[i];
        return i;
      }
    }
    localTriangleIndex =
        triangleIndex - m_verticesScan[m_verticesScan.size() - 1];
    return m_verticesScan.size() - 1;
  }

  std::vector<MortonCode> mortons;
  std::vector<BVHNode> internalNodes;
  std::vector<glm::vec3> treeAABBs;
  std::vector<unsigned int> indices;
  std::vector<glm::vec3> points;

  // init multi temporary hack
  std::vector<int> m_verticesScan;
};

class BuildNodes {
public:
  BuildNodes(std::vector<MortonCode> *mortons,
             std::vector<BVHNode> *internalNodes)

  {
    m_mortons = mortons;
    m_internalNodes = internalNodes;
  }

  void operator()(const tbb::blocked_range<size_t> &r) const;

private:
  std::vector<MortonCode> *m_mortons;
  std::vector<BVHNode> *m_internalNodes;
};

class BuildNodesAABBs {
public:
  BuildNodesAABBs(const MortonCode *mortons, const BVHNode *internalNodes,
                  const glm::vec3 *points, glm::vec3 *treeAABBs) {
    m_mortons = mortons;
    m_internalNodes = internalNodes;
    m_points = points;
    m_treeAABBs = (treeAABBs);
  }

  void operator()(const tbb::blocked_range<size_t> &r) const;

public:
  static const float AABB_PADDING;

private:
  const MortonCode *m_mortons;
  const BVHNode *m_internalNodes;
  const glm::vec3 *m_points;
  glm::vec3 *m_treeAABBs;
};
class BuildRootAABB {
public:
  BuildRootAABB(glm::vec3 *minP, glm::vec3 *maxP,
                const std::vector<glm::vec3> *points, tbb::spin_mutex *mutex) {
    m_minP = minP;
    m_maxP = maxP;
    m_points = points;
    m_mutex = mutex;
  }
  void operator()(const tbb::blocked_range<size_t> &r) const;

private:
  const std::vector<glm::vec3> *m_points;
  glm::vec3 *m_minP;
  glm::vec3 *m_maxP;
  tbb::spin_mutex *m_mutex;
};

class BuildCentroids {
public:
  BuildCentroids(glm::vec3 &minP, glm::vec3 &maxP,
                 const std::vector<glm::vec3> *points,
                 const std::vector<unsigned int> *indices,
                 std::vector<MortonCode> *mortons) {
    m_minP = minP;
    m_maxP = maxP;
    m_points = points;
    m_indices = indices;
    m_mortons = mortons;
  }
  void operator()(const tbb::blocked_range<size_t> &r) const;

private:
  glm::vec3 m_minP;
  glm::vec3 m_maxP;
  const std::vector<glm::vec3> *m_points;
  const std::vector<unsigned int> *m_indices;
  std::vector<MortonCode> *m_mortons;
};
/*
class SplatMesh {
public:
  SplatMesh(glm::vec3 &minP, glm::vec3 &maxP,
            const std::vector<glm::vec3> &points, tbb::spin_mutex &mutex)
      : m_minP(minP), m_maxP(maxP), m_points(points), m_mutex(mutex) {}
  void operator()(const tbb::blocked_range<size_t> &r) const;

private:
  const std::vector<glm::vec3> &m_points;
  glm::vec3 &m_minP;
  glm::vec3 &m_maxP;
  tbb::spin_mutex &m_mutex;
};
*/

} // namespace core
} // namespace mg_ray
