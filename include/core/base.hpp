#pragma once
#include <glm/glm.hpp>
#include <string>
template <typename T>
struct Point3
{
    T x, y, z;
    Point3() : x(0), y(0), z(0) {}
    Point3(T x) : x(x), y(x), z(x) {}
    Point3(T x, T y, T z) : x(x), y(y), z(z) {}
    Point3 operator+(const Point3 &p) const
    {
        return Point3(x + p.x, y + p.y, z + p.z);
    }
    Point3 operator-(const Point3 &p) const
    {
        return Point3(x - p.x, y - p.y, z - p.z);
    }
    std::string toString() const
    {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

template <typename T>
Point3<T> Min(const Point3<T> &p1, const Point3<T> &p2)
{
    return Point3<T>(p1.x < p2.x ? p1.x : p2.x, p1.y < p2.y ? p1.y : p2.y, p1.z < p2.z ? p1.z : p2.z);
}

template <typename T>
Point3<T> Max(const Point3<T> &p1, const Point3<T> &p2)
{
    return Point3<T>(p1.x > p2.x ? p1.x : p2.x, p1.y > p2.y ? p1.y : p2.y, p1.z > p2.z ? p1.z : p2.z);
}

template <typename T>
struct BoundingBox3
{
    Point3<T> pMin, pMax;
    BoundingBox3() : pMin(std::numeric_limits<T>::infinity()), pMax(-std::numeric_limits<T>::infinity()) {}
    BoundingBox3(const Point3<T> &pMin_, const Point3<T> &pMax_) : pMin(pMin_), pMax(pMax_) {}
    void implementTransform(const glm::mat4 &m)
    {
        glm::vec4 newPMin = m * glm::vec4(pMin.x, pMin.y, pMin.z, 1.0f);
        glm::vec4 newPMax = m * glm::vec4(pMax.x, pMax.y, pMax.z, 1.0f);
        pMinNDC = Point3<T>(newPMin.x / newPMin.w, newPMin.y / newPMin.w, newPMin.z / newPMin.w);
        pMaxNDC = Point3<T>(newPMax.x / newPMax.w, newPMax.y / newPMax.w, newPMax.z / newPMax.w);
        minZ = pMinNDC.z > pMaxNDC.z ? pMaxNDC.z : pMinNDC.z;
    }

    float getMinZ() const
    {
        return minZ;
    }

    Point3<T> Diagonal() const
    {
        return pMax - pMin;
    }

    int MaximumExtent() const
    {
        Point3<T> d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    std::string toString() const
    {
        return "pMin: " + pMin.toString() + " pMax: " + pMax.toString();
    }

private:
    Point3<T> pMinNDC,
        pMaxNDC;
    float minZ;
};

using Point3f = Point3<float>;
using BoundingBox3f = BoundingBox3<float>;

template <typename T>
BoundingBox3<T> Union(const BoundingBox3<T> &b, const Point3<T> &p)
{
    Bounds3<T> ret;
    ret.pMin = Min(b.pMin, p);
    ret.pMax = Max(b.pMax, p);
    return ret;
}

template <typename T>
BoundingBox3<T> Union(const BoundingBox3<T> &b1, const BoundingBox3<T> &b2)
{
    BoundingBox3<T> ret;
    ret.pMin = Min(b1.pMin, b2.pMin);
    ret.pMax = Max(b1.pMax, b2.pMax);
    return ret;
}
