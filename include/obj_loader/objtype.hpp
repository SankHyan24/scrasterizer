#pragma once
#include <glm/glm.hpp>
using Float32 = float;
using Uint = unsigned int;
using VertexIndex = int;
using NormalIndex = int;
using TextureUVIndex = int;

struct Vertex
{
    Float32 x, y, z;
    Vertex(Float32 x, Float32 y, Float32 z) : x(x), y(y), z(z) {}
    Vertex(const glm::vec3 &v) : x(v.x), y(v.y), z(v.z) {}
};

struct Normal
{
    Float32 nx, ny, nz;
    Normal(Float32 nx, Float32 ny, Float32 nz) : nx(nx), ny(ny), nz(nz) {}
};

struct TexutreUV
{
    Float32 u, v;
    TexutreUV(Float32 u, Float32 v) : u(u), v(v) {}
};

struct Face
{
    VertexIndex v0, v1, v2;
    TextureUVIndex uv0, uv1, uv2;
    NormalIndex n0, n1, n2;
    Face(VertexIndex v0, VertexIndex v1, VertexIndex v2,
         TextureUVIndex uv0, TextureUVIndex uv1, TextureUVIndex uv2,
         NormalIndex n0, NormalIndex n1, NormalIndex n2)
        : v0(v0), v1(v1), v2(v2),
          uv0(uv0), uv1(uv1), uv2(uv2),
          n0(n0), n1(n1), n2(n2) {}
    Face() : v0(-1), v1(-1), v2(-1),
             uv0(-1), uv1(-1), uv2(-1),
             n0(-1), n1(-1), n2(-1) {}
};