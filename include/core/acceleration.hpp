#pragma once
#include <memory>

#include <core/base.hpp>
class AccelerationStructure
{
public:
    virtual ~AccelerationStructure() = default;

    virtual void build() = 0;
    virtual void intersect() = 0;
};

struct BVHBuildNode
{
    BoundingBox3f bounds;
    BVHBuildNode *children[2];
    int splitAxis; // leaf node set as 3
    int firstPrimOffset;
    int nPrimitives;

    void initLeaf(int first, int n, const BoundingBox3f &b)
    {
        // std::cout << "initLeaf" << std::endl;
        // std::cout << "first: " << first << " n: " << n << std::endl;
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = std::move(b);
        children[0] = children[1] = nullptr;
        splitAxis = 3;
    }

    void initInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1)
    {
        // std::cout << "initInterior" << std::endl;
        // std::cout << "axis: " << axis << " c0: " << c0->toString() << " c1: " << c1->toString() << std::endl;
        children[0] = c0;
        children[1] = c1;
        bounds = Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }

    // to string
    std::string toString()
    {
        return bounds.toString();
    }
};