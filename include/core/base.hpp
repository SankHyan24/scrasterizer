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
    T operator[](int i) const
    {
        return i == 0 ? x : (i == 1 ? y : z);
    }
    friend std::ostream &operator<<(std::ostream &os, const Point3 &p)
    {
        os << p.toString();
        return os;
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
    BoundingBox3() : pMin(std::numeric_limits<T>::infinity()), pMax(-std::numeric_limits<T>::infinity()), minZ(std::numeric_limits<T>::infinity()) {}
    BoundingBox3(const Point3<T> &pMin_, const Point3<T> &pMax_) : pMin(pMin_), pMax(pMax_) {}
    void implementTransform(const glm::mat4 &m) // calculate minZ
    {
        glm::vec4 v0(pMin.x, pMin.y, pMin.z, 1.0f);
        glm::vec4 v1(pMax.x, pMin.y, pMin.z, 1.0f);
        glm::vec4 v2(pMax.x, pMax.y, pMin.z, 1.0f);
        glm::vec4 v3(pMin.x, pMax.y, pMin.z, 1.0f);
        glm::vec4 v4(pMin.x, pMin.y, pMax.z, 1.0f);
        glm::vec4 v5(pMax.x, pMin.y, pMax.z, 1.0f);
        glm::vec4 v6(pMax.x, pMax.y, pMax.z, 1.0f);
        glm::vec4 v7(pMin.x, pMax.y, pMax.z, 1.0f);
        v0 = m * v0;
        v1 = m * v1;
        v2 = m * v2;
        v3 = m * v3;
        v4 = m * v4;
        v5 = m * v5;
        v6 = m * v6;
        v7 = m * v7;
        v0 /= v0.w;
        minZ = v0.z;
        pMinNew.x = v0.x;
        pMinNew.y = v0.y;
        pMinNew.z = v0.z;
        pMaxNew.x = v0.x;
        pMaxNew.y = v0.y;
        pMaxNew.z = v0.z;
        v1 /= v1.w;
        minZ = minZ < v1.z ? minZ : v1.z;
        pMinNew.x = pMinNew.x < v1.x ? pMinNew.x : v1.x;
        pMinNew.y = pMinNew.y < v1.y ? pMinNew.y : v1.y;
        pMinNew.z = pMinNew.z < v1.z ? pMinNew.z : v1.z;
        pMaxNew.x = pMaxNew.x > v1.x ? pMaxNew.x : v1.x;
        pMaxNew.y = pMaxNew.y > v1.y ? pMaxNew.y : v1.y;
        pMaxNew.z = pMaxNew.z > v1.z ? pMaxNew.z : v1.z;
        v2 /= v2.w;
        minZ = minZ < v2.z ? minZ : v2.z;
        pMinNew.x = pMinNew.x < v2.x ? pMinNew.x : v2.x;
        pMinNew.y = pMinNew.y < v2.y ? pMinNew.y : v2.y;
        pMinNew.z = pMinNew.z < v2.z ? pMinNew.z : v2.z;
        pMaxNew.x = pMaxNew.x > v2.x ? pMaxNew.x : v2.x;
        pMaxNew.y = pMaxNew.y > v2.y ? pMaxNew.y : v2.y;
        pMaxNew.z = pMaxNew.z > v2.z ? pMaxNew.z : v2.z;
        v3 /= v3.w;
        minZ = minZ < v3.z ? minZ : v3.z;
        pMinNew.x = pMinNew.x < v3.x ? pMinNew.x : v3.x;
        pMinNew.y = pMinNew.y < v3.y ? pMinNew.y : v3.y;
        pMinNew.z = pMinNew.z < v3.z ? pMinNew.z : v3.z;
        pMaxNew.x = pMaxNew.x > v3.x ? pMaxNew.x : v3.x;
        pMaxNew.y = pMaxNew.y > v3.y ? pMaxNew.y : v3.y;
        pMaxNew.z = pMaxNew.z > v3.z ? pMaxNew.z : v3.z;
        v4 /= v4.w;
        minZ = minZ < v4.z ? minZ : v4.z;
        pMinNew.x = pMinNew.x < v4.x ? pMinNew.x : v4.x;
        pMinNew.y = pMinNew.y < v4.y ? pMinNew.y : v4.y;
        pMinNew.z = pMinNew.z < v4.z ? pMinNew.z : v4.z;
        pMaxNew.x = pMaxNew.x > v4.x ? pMaxNew.x : v4.x;
        pMaxNew.y = pMaxNew.y > v4.y ? pMaxNew.y : v4.y;
        pMaxNew.z = pMaxNew.z > v4.z ? pMaxNew.z : v4.z;
        v5 /= v5.w;
        minZ = minZ < v5.z ? minZ : v5.z;
        pMinNew.x = pMinNew.x < v5.x ? pMinNew.x : v5.x;
        pMinNew.y = pMinNew.y < v5.y ? pMinNew.y : v5.y;
        pMinNew.z = pMinNew.z < v5.z ? pMinNew.z : v5.z;
        pMaxNew.x = pMaxNew.x > v5.x ? pMaxNew.x : v5.x;
        pMaxNew.y = pMaxNew.y > v5.y ? pMaxNew.y : v5.y;
        pMaxNew.z = pMaxNew.z > v5.z ? pMaxNew.z : v5.z;
        v6 /= v6.w;
        minZ = minZ < v6.z ? minZ : v6.z;
        pMinNew.x = pMinNew.x < v6.x ? pMinNew.x : v6.x;
        pMinNew.y = pMinNew.y < v6.y ? pMinNew.y : v6.y;
        pMinNew.z = pMinNew.z < v6.z ? pMinNew.z : v6.z;
        pMaxNew.x = pMaxNew.x > v6.x ? pMaxNew.x : v6.x;
        pMaxNew.y = pMaxNew.y > v6.y ? pMaxNew.y : v6.y;
        pMaxNew.z = pMaxNew.z > v6.z ? pMaxNew.z : v6.z;
        v7 /= v7.w;
        minZ = minZ < v7.z ? minZ : v7.z;
        pMinNew.x = pMinNew.x < v7.x ? pMinNew.x : v7.x;
        pMinNew.y = pMinNew.y < v7.y ? pMinNew.y : v7.y;
        pMinNew.z = pMinNew.z < v7.z ? pMinNew.z : v7.z;
        pMaxNew.x = pMaxNew.x > v7.x ? pMaxNew.x : v7.x;
        pMaxNew.y = pMaxNew.y > v7.y ? pMaxNew.y : v7.y;
        pMaxNew.z = pMaxNew.z > v7.z ? pMaxNew.z : v7.z;
    }

    float getMinZ() const { return minZ; }

    Point3<T> Diagonal() const { return pMax - pMin; }

    T SurfaceArea() const
    {
        Point3<T> d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
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

    float minZ;
    Point3<T> pMinNew, pMaxNew;
};

using Point3f = Point3<float>;
using BoundingBox3f = BoundingBox3<float>;

template <typename T>
BoundingBox3<T> Union(const BoundingBox3<T> &b, const Point3<T> &p)
{
    BoundingBox3<T> ret;
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
