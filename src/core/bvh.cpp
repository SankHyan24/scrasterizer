#include <iostream>

#include <core/bvh.hpp>

void RasterBVH::implementTransform(glm::mat4 &m)
{
    assert(_root != nullptr && "BVH root is nullptr");
    if (_root == nullptr)
        return;
    __implementTransform(m, _root);
}

void RasterBVH::traversalBVH()
{
    assert(_root != nullptr && "BVH root is nullptr");
    if (_root == nullptr)
        return;
    __traversalBVH(_root);
}

void RasterBVH::__initBVH()
{
    _depth = 0;
    _faceInfo.resize(_faces.size());
    for (Uint i = 0; i < _faces.size(); i++)
        _faceInfo[i] = i;
    Uint offset = 0;
    if (_method == SplitMethod::SAH)
    {
        _root = __recursiveBuildSAH(0, _faces.size());
    }
    else
    {
        _root = __recursiveBuild(0, _faces.size());
    }
}

BVHBuildNode *RasterBVH::__recursiveBuild(int start, int end)
{
    // std::cout << "start: " << start << " end: " << end << std::endl;
    _depth++;
    BVHBuildNode *node = _arena->Alloc<BVHBuildNode>();
    _totalNodes++;
    float minX, minY, minZ, maxX, maxY, maxZ;
    minX = minY = minZ = std::numeric_limits<float>::infinity();
    maxX = maxY = maxZ = -std::numeric_limits<float>::infinity();
    for (int i = start; i < end; i++)
    {
        auto &face = _faces[i];

        auto &v0 = _vertices[face.v0];
        auto &v1 = _vertices[face.v1];
        auto &v2 = _vertices[face.v2];
        minX = minX < v0.x ? minX : v0.x;
        minX = minX < v1.x ? minX : v1.x;
        minX = minX < v2.x ? minX : v2.x;
        minY = minY < v0.y ? minY : v0.y;
        minY = minY < v1.y ? minY : v1.y;
        minY = minY < v2.y ? minY : v2.y;
        minZ = minZ < v0.z ? minZ : v0.z;
        minZ = minZ < v1.z ? minZ : v1.z;
        minZ = minZ < v2.z ? minZ : v2.z;
        maxX = maxX > v0.x ? maxX : v0.x;
        maxX = maxX > v1.x ? maxX : v1.x;
        maxX = maxX > v2.x ? maxX : v2.x;
        maxY = maxY > v0.y ? maxY : v0.y;
        maxY = maxY > v1.y ? maxY : v1.y;
        maxY = maxY > v2.y ? maxY : v2.y;
        maxZ = maxZ > v0.z ? maxZ : v0.z;
        maxZ = maxZ > v1.z ? maxZ : v1.z;
        maxZ = maxZ > v2.z ? maxZ : v2.z;
    }
    float min_tmp[3] = {minX, minY, minZ};
    float max_tmp[3] = {maxX, maxY, maxZ};
    BoundingBox3f bounds(Point3f(minX, minY, minZ), Point3f(maxX, maxY, maxZ));
    // std::cout << bounds.toString() << std::endl;
    int dim = bounds.MaximumExtent();
    Uint nPrimitives = end - start;
    node->nPrimitives = nPrimitives;
    int mid = (start + end) / 2;
    auto &&diagonal = (bounds.Diagonal());

    // if the length of the bound is less than the minimum bound length, expand the bound to the minimum bound length
    if (diagonal.x < _minBoundLength && diagonal.y < _minBoundLength && diagonal.z < _minBoundLength)
    {
        bounds.pMax.x += _voxel_length;
        bounds.pMax.y += _voxel_length;
        bounds.pMax.z += _voxel_length;
        bounds.pMin.x -= _voxel_length;
        bounds.pMin.y -= _voxel_length;
        bounds.pMin.z -= _voxel_length;

        Uint firstposition = _orderdata.size();
        for (Uint i = start; i < end; i++)
        {
            _orderdata.push_back(_faceInfo[i]);
        }
        node->initLeaf(firstposition, nPrimitives, bounds);
        _depth--;
        return node;
    }

    // if only one primitive, set the node as a leaf node
    if (nPrimitives == 1)
    {
        Uint firstposition = _orderdata.size();
        _orderdata.push_back(_faceInfo[start]);
        node->initLeaf(firstposition, 1, bounds);
        _depth--;
        return node;
    }

    // for internal nodes
    switch (_method)
    {
    case SplitMethod::Middle:
    {
        float pmid = (min_tmp[dim] + max_tmp[dim]) / 2;
        auto miditer = std::partition(&_faceInfo[start], &_faceInfo[end - 1] + 1,
                                      [dim, pmid, this](Uint faceIndex)
                                      {
                                          auto &face = _faces[faceIndex];
                                          auto &v0 = _vertices[face.v0];
                                          auto &v1 = _vertices[face.v1];
                                          auto &v2 = _vertices[face.v2];
                                          float centroid = (v0[dim] + v1[dim] + v2[dim]) / 3;
                                          return centroid < pmid;
                                      });
        mid = miditer - &_faceInfo[0];
    };
    break;
    case SplitMethod::EqualCounts:
    {
        std::nth_element(&_faceInfo[start], &_faceInfo[mid], &_faceInfo[end - 1] + 1,
                         [dim, this](Uint faceIndex1, Uint faceIndex2)
                         {
                             auto &face1 = _faces[faceIndex1];
                             auto &v01 = _vertices[face1.v0];
                             auto &v11 = _vertices[face1.v1];
                             auto &v21 = _vertices[face1.v2];
                             auto &face2 = _faces[faceIndex2];
                             auto &v02 = _vertices[face2.v0];
                             auto &v12 = _vertices[face2.v1];
                             auto &v22 = _vertices[face2.v2];
                             return (v01[dim] + v11[dim] + v21[dim]) / 3 <
                                    (v02[dim] + v12[dim] + v22[dim]) / 3;
                         });
    };
    break;
    default:
        assert(false && "SplitMethod not implemented");
    }
    // std::cout << "dim: " << dim << " mid: " << mid << std::endl;
    node->initInterior(dim, __recursiveBuild(start, mid), __recursiveBuild(mid, end));
    _depth--;
    return node;
}

BVHBuildNode *RasterBVH::__recursiveBuildSAH(int start, int end)
{
    BVHBuildNode *node = _arena->Alloc<BVHBuildNode>();
    _totalNodes++;
    float minX, minY, minZ, maxX, maxY, maxZ;
    minX = minY = minZ = std::numeric_limits<float>::infinity();
    maxX = maxY = maxZ = -std::numeric_limits<float>::infinity();
    for (int i = start; i < end; i++)
    {
        auto &face = _faces[i];

        auto &v0 = _vertices[face.v0];
        auto &v1 = _vertices[face.v1];
        auto &v2 = _vertices[face.v2];
        minX = minX < v0.x ? minX : v0.x;
        minX = minX < v1.x ? minX : v1.x;
        minX = minX < v2.x ? minX : v2.x;
        minY = minY < v0.y ? minY : v0.y;
        minY = minY < v1.y ? minY : v1.y;
        minY = minY < v2.y ? minY : v2.y;
        minZ = minZ < v0.z ? minZ : v0.z;
        minZ = minZ < v1.z ? minZ : v1.z;
        minZ = minZ < v2.z ? minZ : v2.z;
        maxX = maxX > v0.x ? maxX : v0.x;
        maxX = maxX > v1.x ? maxX : v1.x;
        maxX = maxX > v2.x ? maxX : v2.x;
        maxY = maxY > v0.y ? maxY : v0.y;
        maxY = maxY > v1.y ? maxY : v1.y;
        maxY = maxY > v2.y ? maxY : v2.y;
        maxZ = maxZ > v0.z ? maxZ : v0.z;
        maxZ = maxZ > v1.z ? maxZ : v1.z;
        maxZ = maxZ > v2.z ? maxZ : v2.z;
    }
    BoundingBox3f bounds(Point3f(minX, minY, minZ), Point3f(maxX, maxY, maxZ));
    int dim = bounds.MaximumExtent();
    Uint nPrimitives = end - start;
    node->nPrimitives = nPrimitives;
    int mid = (start + end) / 2;
    auto &&diagonal = (bounds.Diagonal());

    if (diagonal.x < _minBoundLength && diagonal.y < _minBoundLength && diagonal.z < _minBoundLength)
    {
        bounds.pMax.x += _voxel_length;
        bounds.pMax.y += _voxel_length;
        bounds.pMax.z += _voxel_length;
        bounds.pMin.x -= _voxel_length;
        bounds.pMin.y -= _voxel_length;
        bounds.pMin.z -= _voxel_length;

        Uint firstposition = _orderdata.size();
        for (Uint i = start; i < end; i++)
        {
            _orderdata.push_back(_faceInfo[i]);
        }
        node->initLeaf(firstposition, nPrimitives, bounds);
        return node;
    }
    return node;
}

void RasterBVH::__traversalBVH(BVHBuildNode *node)
{
    if (node->splitAxis == 3)
    {
        // leaf node
        _traversalDebug(node->bounds);
    }
    else
    {
        // internal node
        __traversalBVH(node->children[0]);
        __traversalBVH(node->children[1]);
    }
}

void RasterBVH::__printBVHInfo()
{
    std::cout << "BVH Info: " << std::endl;
    std::cout << "Total Nodes: " << _totalNodes << std::endl;
    std::cout << "Depth: " << _depth << std::endl;
}

void RasterBVH::__implementTransform(glm::mat4 &m, BVHBuildNode *node)
{
    // calculate z value
    if (node->splitAxis == 3)
    {
        node->bounds.implementTransform(m);
    }
    else
    {
        __implementTransform(m, node->children[0]);
        __implementTransform(m, node->children[1]);
    }
}
