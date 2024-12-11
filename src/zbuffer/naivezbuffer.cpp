#include <zbuffer/naivezbuffer.hpp>

NaiveZBuffer::NaiveZBuffer(int width, int height) : width(width), height(height)
{
    zBufferData = new float[width * height];
    colorBufferData = new float[width * height * 3];
}

NaiveZBuffer::~NaiveZBuffer()
{
    delete[] zBufferData;
    delete[] colorBufferData;
}

void NaiveZBuffer::init()
{
    std::fill(zBufferData, zBufferData + width * height, std::numeric_limits<float>::infinity());
    std::fill(colorBufferData, colorBufferData + width * height * 3, 0.0f);
    // clear
    vertices.clear();
    faces.clear();
    triangles.clear();
    obj_vertex_offset = 0;
    culled_face = 0;
}

void NaiveZBuffer::prepareVertex(OBJ &obj, Camera &camera)
{
    __vertexShader(obj, camera);
}

void NaiveZBuffer::drawFragment()
{
    __fragmentShader();
}

void NaiveZBuffer::__vertexShader(OBJ &obj, Camera &camera)
{
    // from world space to screen space
    auto &faces_ = obj.getFaces();
    auto &vertices_ = obj.getVertices();
    glm::mat4 MVP = camera.getViewProjectionMatrix();
    for (auto &v : vertices_)
    {
        glm::vec4 v4(v.x, v.y, v.z, 1.0f);
        v4 = MVP * v4;
        Vertex vertex(v4.x / v4.w, v4.y / v4.w, v4.z / v4.w);
        vertex.x = (vertex.x + 1.0f) * width / 2.0f;
        vertex.y = (vertex.y + 1.0f) * height / 2.0f;
        vertex.nx = v.nx;
        vertex.ny = v.ny;
        vertex.nz = v.nz;
        // vertex.nx = std::fabs(v.nx);
        // vertex.ny = std::fabs(v.ny);
        // vertex.nz = std::fabs(v.nz);
        vertices.push_back(vertex);
    }
    // build triangles
    for (int face_index = 0; face_index < faces_.size(); face_index++)
    {
        auto &face = faces_[face_index];
        faces.push_back(face);
        Triangle triangle;
        triangle.v0 = face.v0;
        triangle.v1 = face.v1;
        triangle.v2 = face.v2;
        triangle.v0 = face.v0 + obj_vertex_offset;
        triangle.v1 = face.v1 + obj_vertex_offset;
        triangle.v2 = face.v2 + obj_vertex_offset;
        // bounding box
        auto &v0 = vertices[triangle.v0];
        auto &v1 = vertices[triangle.v1];
        auto &v2 = vertices[triangle.v2];
        triangle.bb.xMin = std::floor(v0.x < v1.x ? (v0.x < v2.x ? v0.x : v2.x) : (v1.x < v2.x ? v1.x : v2.x));
        triangle.bb.xMax = std::ceil(v0.x > v1.x ? (v0.x > v2.x ? v0.x : v2.x) : (v1.x > v2.x ? v1.x : v2.x));
        triangle.bb.yMin = std::floor(v0.y < v1.y ? (v0.y < v2.y ? v0.y : v2.y) : (v1.y < v2.y ? v1.y : v2.y));
        triangle.bb.yMax = std::ceil(v0.y > v1.y ? (v0.y > v2.y ? v0.y : v2.y) : (v1.y > v2.y ? v1.y : v2.y));
        if (__ifTriangleInScreen(triangle))
            triangles.push_back(triangle);
    }
    obj_vertex_offset += vertices_.size();
}

void NaiveZBuffer::__fragmentShader()
{
    for (auto &triangle : triangles)
    {
        float area = triangle.calculateArea(vertices);
        if (area == 0.0f)
            continue;
        __drawTriangle(triangle);
    }
    if (use_cull_face)
        culled_face = faces.size() - triangles.size();
}

bool NaiveZBuffer::__ifLambda12InsideTriangle(float l1, float l2)
{
    return l1 >= 0 && l2 >= 0 && 1 - l1 - l2 >= 0;
}

void NaiveZBuffer::__drawTriangle(const Triangle &triangle)
{
    int xStart = 0 < triangle.bb.xMin ? triangle.bb.xMin : 0;
    int xEnd = width - 1 < triangle.bb.xMax ? width - 1 : triangle.bb.xMax;
    int yStart = 0 < triangle.bb.yMin ? triangle.bb.yMin : 0;
    int yEnd = height - 1 < triangle.bb.yMax ? height - 1 : triangle.bb.yMax;
#pragma omp parallel for collapse(2)
    for (int x = xStart; x <= xEnd; x++)
        for (int y = yStart; y <= yEnd; y++)
        {
            float lambda1, lambda2, lambda3;
            __ComputeBarycentricCoords(x, y, triangle, lambda2, lambda3);
            lambda1 = 1 - lambda2 - lambda3;
            if (__ifLambda12InsideTriangle(lambda1, lambda2))
            {
                float z = lambda1 * vertices[triangle.v0].z + lambda2 * vertices[triangle.v1].z + lambda3 * vertices[triangle.v2].z;
                float R = lambda1 * vertices[triangle.v0].nx + lambda2 * vertices[triangle.v1].nx + lambda3 * vertices[triangle.v2].nx;
                float G = lambda1 * vertices[triangle.v0].ny + lambda2 * vertices[triangle.v1].ny + lambda3 * vertices[triangle.v2].ny;
                float B = lambda1 * vertices[triangle.v0].nz + lambda2 * vertices[triangle.v1].nz + lambda3 * vertices[triangle.v2].nz;
#pragma omp critical
                if (z < zBufferData[y * width + x])
                {
                    zBufferData[y * width + x] = z;
                    colorBufferData[(y * width + x) * 3] = R;
                    colorBufferData[(y * width + x) * 3 + 1] = G;
                    colorBufferData[(y * width + x) * 3 + 2] = B;
                }
            }
        }
}

void NaiveZBuffer::__drawTriangleFrame(const Triangle &triangle)
{
    auto &v0 = vertices[triangle.v0];
    auto &v1 = vertices[triangle.v1];
    auto &v2 = vertices[triangle.v2];

    // draw line first
    __drawLineScreenSpace(glm::vec2(v0.x, v0.y), glm::vec2(v1.x, v1.y), 255, 255, 255);
    __drawLineScreenSpace(glm::vec2(v1.x, v1.y), glm::vec2(v2.x, v2.y), 255, 255, 255);
    __drawLineScreenSpace(glm::vec2(v2.x, v2.y), glm::vec2(v0.x, v0.y), 255, 255, 255);
}

void NaiveZBuffer::__drawTriangleBB(const Triangle &triangle)
{
    // draw bounding box
    auto &BB = triangle.bb;
    // use _drawLineScreenSpace
    auto p0 = glm::vec2(BB.xMin, BB.yMin);
    auto p1 = glm::vec2(BB.xMax, BB.yMin);
    auto p2 = glm::vec2(BB.xMax, BB.yMax);
    auto p3 = glm::vec2(BB.xMin, BB.yMax);
    __drawLineScreenSpace(p0, p1, 1, 1, 1);
    __drawLineScreenSpace(p1, p2, 1, 1, 1);
    __drawLineScreenSpace(p2, p3, 1, 1, 1);
}

bool NaiveZBuffer::__ifTriangleInScreen(const Triangle &triangle)
{
    if (!use_cull_face)
        return true;
    // cull face
    auto &BB = triangle.bb;
    if (BB.xMin >= width || BB.xMax < 0 || BB.yMin >= height || BB.yMax < 0)
        return false;
    return true;
}

void NaiveZBuffer::__drawLineScreenSpace(const glm::vec2 &p0, const glm::vec2 &p1, float r, float g, float b)
{
    auto textureMap = colorBufferData;

    // Bresenham's line algorithm
    int x0 = p0.x, y0 = p0.y;
    int x1 = p1.x, y1 = p1.y;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true)
    {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
        {
            textureMap[(y0 * width + x0) * 3] = r;
            textureMap[(y0 * width + x0) * 3 + 1] = g;
            textureMap[(y0 * width + x0) * 3 + 2] = b;
        }
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void NaiveZBuffer::__ComputeBarycentricCoords(int x, int y, const Triangle &triangle, float &lambda1, float &lambda2)
{
    // barycentric coordinate
    auto &v0 = vertices[triangle.v0];
    auto &v1 = vertices[triangle.v1];
    auto &v2 = vertices[triangle.v2];
    float area = 0.5f * (-v1.y * v2.x + v0.y * (-v1.x + v2.x) + v0.x * (v1.y - v2.y) + v1.x * v2.y);
    lambda1 = 1 / (2 * area) * (v0.y * v2.x - v0.x * v2.y + (v2.y - v0.y) * x + (v0.x - v2.x) * y);
    lambda2 = 1 / (2 * area) * (v0.x * v1.y - v0.y * v1.x + (v0.y - v1.y) * x + (v1.x - v0.x) * y);
}