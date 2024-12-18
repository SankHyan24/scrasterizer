#pragma once
#include <core/acceleration.hpp>
#include <core/memory.hpp>
#include <obj_loader/objtype.hpp>
#include <functional>
using BVHRenderCallBack = std::function<bool(BVHBuildNode *)>; // render callback
using BVHDebugCallBack = std::function<bool(BoundingBox3f)>;

struct BucketInfo
{
    int count = 0;
    BoundingBox3f bounds;
};
class RasterBVH
{
public:
    enum class SplitMethod
    {
        SAH,
        Middle,
        EqualCounts
    };

    //  SplitMethod to string
    static std::string SplitMethodToString(SplitMethod method)
    {
        switch (method)
        {
        case SplitMethod::SAH:
            return "SAH";
        case SplitMethod::Middle:
            return "Middle";
        case SplitMethod::EqualCounts:
            return "EqualCounts";
        default:
            return "Unknown";
        }
    }

    const double _voxel_length;   // voxel length
    const double _minBoundLength; // minimum bound length, if the length of a bound is less than this value, the bound will be expanded to this value
    const SplitMethod _method;
    std::unique_ptr<MemoryArena> _arena;
    std::vector<Face> &_faces;
    std::vector<Vertex> &_vertices;
    std::vector<Uint> _orderdata;
    std::vector<Uint> _faceInfo;
    Uint _totalNodes{0};
    BVHBuildNode *_root{nullptr};
    Uint _memorySize{0};
    BoundingBox3f _sceneBoundCurrent;
    int _depth{0};
    int _maxDepth{0};

    BVHRenderCallBack _traversalRenderCallback;
    BVHDebugCallBack _traversalDebug; // draw bb debug
    RasterBVH(std::vector<Face> &faces, std::vector<Vertex> &vertices, double voxel_length, double minBoundLength, SplitMethod method = RasterBVH::SplitMethod::Middle, Uint memorySize = 128)
        : _faces(faces), _vertices(vertices), _voxel_length(voxel_length), _minBoundLength(minBoundLength), _method(method), _memorySize(memorySize)
    {
        _arena = std::make_unique<MemoryArena>(memorySize * 1024 * 1024);
        // std::cout << "BVH voxel length: " << voxel_length << " minBoundLength: " << minBoundLength << " memorySize: " << memorySize << "MB" << std::endl;
        __initBVH();
        __printBVHInfo();
    }
    ~RasterBVH() = default;
    void implementTransform(glm::mat4 &m); // called before each render loop
    void traversalBVH();                   // called in the render segment shader

    static BoundingBox3f getBBfromFace(const Face &face, const std::vector<Vertex> &vertices)
    {
        BoundingBox3f bb;
        bb.pMin = std::numeric_limits<float>::infinity();
        bb.pMax = -std::numeric_limits<float>::infinity();
        Point3f p0(vertices[face.v0].x, vertices[face.v0].y, vertices[face.v0].z);
        Point3f p1(vertices[face.v1].x, vertices[face.v1].y, vertices[face.v1].z);
        Point3f p2(vertices[face.v2].x, vertices[face.v2].y, vertices[face.v2].z);
        bb = Union(bb, p0);
        bb = Union(bb, p1);
        bb = Union(bb, p2);
        return bb;
    }

private:
    void __initBVH();

    void __printBVHInfo();
    BVHBuildNode *__recursiveBuild(int start, int end);
    BVHBuildNode *__recursiveBuildSAH(int start, int end);

    void __traversalBVH(BVHBuildNode *node);
    void __implementTransform(glm::mat4 &m, BVHBuildNode *node);
};