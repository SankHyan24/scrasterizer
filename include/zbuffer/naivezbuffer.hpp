#pragma once
#include <rasterizer/rasterizer.hpp>

class NaiveZBuffer // Naive Z-Buffer Rasterizer
{
public:
    struct BoundingBox2D
    {
        int xMin, xMax, yMin, yMax;
    };
    struct Triangle
    {
        VertexIndex v0, v1, v2;
        BoundingBox2D bb;
        float calculateArea(const std::vector<Vertex> &vertices) const
        {
            auto &v0_ = vertices[v0];
            auto &v1_ = vertices[v1];
            auto &v2_ = vertices[v2];
            glm::vec3 e1(v1_.x - v0_.x, v1_.y - v0_.y, v1_.z - v0_.z);
            glm::vec3 e2(v2_.x - v0_.x, v2_.y - v0_.y, v2_.z - v0_.z);
            glm::vec3 cross = glm::cross(e1, e2);
            return 0.5f * glm::length(cross);
        }
    };
    int height, width;
    float *zBufferData{nullptr};
    float *colorBufferData{nullptr};
    bool use_cull_face{true};
    int culled_face{0};

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::vector<Triangle> triangles; // after culled

    NaiveZBuffer(int width, int height);
    ~NaiveZBuffer();

    void init();
    void prepareVertex(OBJ &obj, Camera &camera);
    void drawFragment();

private:
    void __vertexShader(OBJ &obj, Camera &camera);
    void __fragmentShader();

    void __drawTriangle(const Triangle &triangle);
    void __drawTriangleFrame(const Triangle &triangle);
    void __drawTriangleBB(const Triangle &triangle);

    bool __ifTriangleInScreen(const Triangle &triangle);
    static bool __ifLambda12InsideTriangle(float l1, float l2);
    void __drawLineScreenSpace(const glm::vec2 &p0, const glm::vec2 &p1, float r, float g, float b);
    void __ComputeBarycentricCoords(int x, int y, const Triangle &triangle, float &lambda1, float &lambda2);

    int obj_vertex_offset{0}; // for each obj, the offset of vertices
};

class NaiveZBufferRaster : public Rasterizer
{
public:
    NaiveZBufferRaster(int width, int height, bool isGPU = true);
    ~NaiveZBufferRaster() {}
    void render() override;
    void renderGPU() override { assert(false && "NaiveZBufferRaster GPU Not Implemented!"); }
    int renderInit() override { return 1; }
    int renderImGui() override
    {
        _showCameraInfoInImgui();
        _showObjInfosInImgui();
        __showZBufferDataStructInfo();
        return 1;
    }

private:
    void __putColorBuffer2TextureMap();
    std::unique_ptr<NaiveZBuffer> zbuffer;
    float *colorPrecompute{nullptr};

    void __showZBufferDataStructInfo();
};