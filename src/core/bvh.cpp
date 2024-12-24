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

    _context.culledNodes = _totalNodes;
    _context.culledFaces = _context.totalFaces;
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
    _depth++;
    if (_depth > _maxDepth)
        _maxDepth = _depth;
    BVHBuildNode *node = _arena->Alloc<BVHBuildNode>();
    _totalNodes++;
    float minX, minY, minZ, maxX, maxY, maxZ;
    minX = minY = minZ = std::numeric_limits<float>::infinity();
    maxX = maxY = maxZ = -std::numeric_limits<float>::infinity();
    for (int i = start; i < end; i++)
    {
        auto &face = _faces[i];
        auto bb = getBBfromFace(face, _vertices);
        minX = std::min(minX, bb.pMin.x);
        minY = std::min(minY, bb.pMin.y);
        minZ = std::min(minZ, bb.pMin.z);
        maxX = std::max(maxX, bb.pMax.x);
        maxY = std::max(maxY, bb.pMax.y);
        maxZ = std::max(maxZ, bb.pMax.z);
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
        if (start == mid || mid == end)
            mid = (start + end) / 2;
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
    assert(false && "Not implemented correctly");
    _depth++;
    if (_depth > _maxDepth)
        _maxDepth = _depth;
    BVHBuildNode *node = _arena->Alloc<BVHBuildNode>();
    _totalNodes++;
    float minX, minY, minZ, maxX, maxY, maxZ;
    minX = minY = minZ = std::numeric_limits<float>::infinity();
    maxX = maxY = maxZ = -std::numeric_limits<float>::infinity();
    for (int i = start; i < end; i++)
    {
        auto &face = _faces[i];
        auto bb = getBBfromFace(face, _vertices);
        minX = std::min(minX, bb.pMin.x);
        minY = std::min(minY, bb.pMin.y);
        minZ = std::min(minZ, bb.pMin.z);
        maxX = std::max(maxX, bb.pMax.x);
        maxY = std::max(maxY, bb.pMax.y);
        maxZ = std::max(maxZ, bb.pMax.z);
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
    const int nBuckets = 12;
    BucketInfo buckets[nBuckets];
    for (Uint i = start; i < end; i++)
    {
        BoundingBox3f bb = getBBfromFace(_faces[_faceInfo[i]], _vertices);
        int b = nBuckets * (bb.pMin[dim] - min_tmp[dim]) / (max_tmp[dim] - min_tmp[dim]);
        if (b >= nBuckets)
            b = nBuckets - 1;
        buckets[b].count++;
        buckets[b].bounds = Union(buckets[b].bounds, bb);
    }

    float cost[nBuckets];
    for (int i = 0; i < nBuckets; i++)
    {
        BoundingBox3f b0, b1;
        int count0 = 0, count1 = 0;
        for (int j = 0; j <= i; j++)
        {
            b0 = Union(b0, buckets[j].bounds);
            count0 += buckets[j].count;
        }

        for (int j = i + 1; j < nBuckets; j++)
        {
            b1 = Union(b1, buckets[j].bounds);
            count1 += buckets[j].count;
        }

        cost[i] = 1 + ((count0 * b0.SurfaceArea()) + (count1 * b1.SurfaceArea())) / bounds.SurfaceArea();
    }
    float mincost = cost[0];
    int mincostsplit = 0;
    for (int i = 1; i < nBuckets; i++)
    {
        if (cost[i] < mincost)
        {

            std::cout << "cost " << i << ": " << cost[i] << std::endl;
            mincost = cost[i];
            mincostsplit = i;
        }
    }
    float pmid = (bounds.pMax[dim] + bounds.pMin[dim]) / 2;
    std::cout << mincostsplit << std::endl;
    // auto p = std::partition(
    //     &(_faceInfo[start]), &(_faceInfo[end - 1]) + 1,
    //     [&](const auto &faceIndex)
    //     {
    //     auto &face = _faces[faceIndex];
    //     auto bb = getBBfromFace(face, _vertices);
    //     float centroid = (bb.pMin[dim] + bb.pMax[dim]) / 2;
    //     int b = nBuckets * (bb.pMin[dim] - min_tmp[dim]) / (max_tmp[dim] - min_tmp[dim]);
    //     // int b = (nBuckets) * ((centroid - pmid) / (bounds.pMax[dim] - bounds.pMin[dim]));
    //     if (b >= nBuckets)
    //         b = nBuckets - 1;
    //     return b <= mincostsplit; });
    // mid = p - &(_faceInfo[0]);
    if (mid <= start + 1 || mid >= end - 1)
        mid = (start + end) / 2;
    node->initInterior(dim, __recursiveBuildSAH(start, mid), __recursiveBuildSAH(mid, end));
    _depth--;
    return node;
}

void RasterBVH::__traversalBVH(BVHBuildNode *node)
{
    if (node->splitAxis == 3)
    {
        // _traversalDebug(node->bounds);
        _traversalRenderCallback(node, _context);
        _context.culledNodes--;
    }
    else
    {
        // _traversalDebug(node->bounds);
        _traversalRenderCallback(node, _context);
        //  if (!_traversalRenderCallback(node, _context))
        // {
        //     return;
        // }
        _context.culledNodes--;
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
        node->bounds.implementTransform(m);
        __implementTransform(m, node->children[0]);
        __implementTransform(m, node->children[1]);
    }
}
