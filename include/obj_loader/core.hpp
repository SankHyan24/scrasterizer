#pragma once

typedef float Float32;
typedef int VertexIndex;
typedef int NormalIndex;
typedef int TextureUVIndex;

struct Vertex
{
    Float32 x, y, z;
    Vertex(Float32 x, Float32 y, Float32 z) : x(x), y(y), z(z) {}
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
    VertexIndex v1, v2, v3;
    TextureUVIndex uv1, uv2, uv3;
    NormalIndex n1, n2, n3;
};