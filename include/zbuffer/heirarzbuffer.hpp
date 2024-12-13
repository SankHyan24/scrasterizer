#pragma once
#include <zbuffer/naivezbuffer.hpp>
#include <core/bvh.hpp>

class HeirarZBuffer
{
public:
    int height, width;
    float *zBufferData{nullptr};
    float *colorBufferData{nullptr};
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::unique_ptr<RasterBVH> bvh{nullptr};
    glm::mat4 mvp;

    HeirarZBuffer(int width, int height) : width(width), height(height)
    {
        zBufferData = new float[width * height];
        colorBufferData = new float[width * height * 3];
    }
    ~HeirarZBuffer()
    {
        delete[] zBufferData;
        delete[] colorBufferData;
    }

    void init(Camera &camera)
    {
        std::fill(zBufferData, zBufferData + width * height, std::numeric_limits<float>::infinity());
        std::fill(colorBufferData, colorBufferData + width * height * 3, 0.0f);
        mvp = camera.getViewProjectionMatrix(); // model matrix is not available
        bvh->implementTransform(mvp);
    }
    void prepareVertex(OBJ &obj)
    {
        __vertexShader(obj);
    }
    void buildBVH()
    {
        __buildBVH();
        BVHDebugCallBack debug = [this](BoundingBox3f bb) -> bool
        {
            __drawBoundingBox(bb);
            return true;
        };
        bvh->_traversalDebug = debug;
    }
    void drawFragment()
    {
        __fragmentShader();
    }

private:
    void __vertexShader(OBJ &obj)
    {
        // from world space to screen space
        auto &faces_ = obj.getFaces();
        auto &vertices_ = obj.getVertices();
        for (auto &v : vertices_)
            vertices.push_back(v);
        // build triangles
        for (int face_index = 0; face_index < faces_.size(); face_index++)
        {
            auto &face = faces_[face_index];
            Face newFace;
            newFace.v0 = face.v0 + obj_vertex_offset;
            newFace.v1 = face.v1 + obj_vertex_offset;
            newFace.v2 = face.v2 + obj_vertex_offset;
            faces.push_back(newFace);
        }
        obj_vertex_offset += vertices_.size();
    }
    void __buildBVH()
    {
        bvh = std::make_unique<RasterBVH>(faces, vertices, 0.01, 0.01, RasterBVH::SplitMethod::EqualCounts, 256);
    }
    void __fragmentShader()
    {
        bvh->traversalBVH(); // 目前的渲染循环是执行__drawBoundingBox
        // 预期目标是执行__drawFragment。
        // TODO zbuffer需要分区，然后维护minZ和maxZ
        // 还需要实现AABB是否在zbuffer的某个分区内的判断
        // 总的来说是先实现一个拒绝函数。用来判断三角形是否会对当前framebuffer产生影响
        // 根据就是先判断投影到屏幕上的bounding box是否在zbuffer的某个分区内
        // 其次就是判断bounding box的minZ是否小于zbuffer的maxZ。如果是的话，就需要绘制这个三角形
        // 最好还是不要逐像素来判断了，计算量太大了
        // 把画布分成若干个区域，然后对每个区域进行判断最好，这个颗粒度是可以调整的
    }
    void __drawBoundingBox(BoundingBox3f &bb)
    {
        auto &pMin = bb.pMin;
        auto &pMax = bb.pMax;
        auto p0 = glm::vec3(pMin.x, pMin.y, pMin.z);
        auto p1 = glm::vec3(pMax.x, pMin.y, pMin.z);
        auto p2 = glm::vec3(pMax.x, pMax.y, pMin.z);
        auto p3 = glm::vec3(pMin.x, pMax.y, pMin.z);
        auto p4 = glm::vec3(pMin.x, pMin.y, pMax.z);
        auto p5 = glm::vec3(pMax.x, pMin.y, pMax.z);
        auto p6 = glm::vec3(pMax.x, pMax.y, pMax.z);
        auto p7 = glm::vec3(pMin.x, pMax.y, pMax.z);
        __drawLine(p0, p1);
        __drawLine(p1, p2);
        __drawLine(p2, p3);
        __drawLine(p3, p0);
        __drawLine(p4, p5);
        __drawLine(p5, p6);
        __drawLine(p6, p7);
        __drawLine(p7, p4);
        __drawLine(p0, p4);
        __drawLine(p1, p5);
        __drawLine(p2, p6);
        __drawLine(p3, p7);
    }
    void __drawLine(const glm::vec3 &p0, const glm::vec3 &p1)
    {
        // from world space to screen space
        auto p0_ = glm::vec4(p0, 1.0f);
        auto p1_ = glm::vec4(p1, 1.0f);
        p0_ = mvp * p0_;
        p1_ = mvp * p1_;
        p0_ /= p0_.w;
        p1_ /= p1_.w;
        // to screen space
        p0_.x = (p0_.x + 1.0f) * 0.5f * width;
        p0_.y = (1.0f + p0_.y) * 0.5f * height;
        p1_.x = (p1_.x + 1.0f) * 0.5f * width;
        p1_.y = (1.0f + p1_.y) * 0.5f * height;
        glm::vec2 p0_screen(p0_.x, p0_.y);
        glm::vec2 p1_screen(p1_.x, p1_.y);
        __drawLineScreenSpace(p0_screen, p1_screen, 255.0f, 1.0f, 1.0f);
    }
    void __drawLineScreenSpace(const glm::vec2 &p0, const glm::vec2 &p1, float r, float g, float b)
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
    void __drawFragment()
    {
        // to be implemented
    }
    int obj_vertex_offset{0}; // for each obj, the offset of vertices
};

class HeirarZBufferRaster : public Rasterizer
{
public:
    HeirarZBufferRaster(int width, int height, bool isGPU = true) : Rasterizer(width, height, isGPU)
    {
        zbuffer = std::make_unique<HeirarZBuffer>(width, height);
        zBufferPrecompute = zbuffer->zBufferData;
        colorPrecompute = zbuffer->colorBufferData;
    }
    ~HeirarZBufferRaster() {}
    void render() override
    {
        _autoRotateCamera();
        zbuffer->init(scene->getCameraV()); // init zbuffer and implement transform to BVH
        zbuffer->drawFragment();
        __putColorBuffer2TextureMap();
        _drawCoordinateAxis();
    }
    void renderGPU() override { assert(false && "NaiveZBufferRaster GPU Not Implemented!"); }
    int renderInit() override
    {
        for (int i = 0; i < scene->objs.size(); i++)
            // if (scene->obj_activated[i])
            zbuffer->prepareVertex(*scene->objs[i]);
        zbuffer->buildBVH(); // build BVH

        return 1;
    }
    int renderImGui() override
    {
        _showCameraInfoInImgui();
        _showObjInfosInImgui();
        __showHZBufferDataStructInfo();
        return 1;
    }

private:
    void __putColorBuffer2TextureMap()
    {
        auto textureMap = canvas->getTextureMap();
        for (int i = 0; i < width * height * 3; i++)
        {
            float climped = colorPrecompute[i] > 1.0f ? 1.0f : colorPrecompute[i];
            climped = climped < 0.0f ? 0.0f : climped;
            textureMap[i] = (unsigned char)(int)(climped * 255);
        }
    }
    std::unique_ptr<HeirarZBuffer> zbuffer;
    float *colorPrecompute{nullptr};

    void __showHZBufferDataStructInfo()
    {
        _showimguiSubTitle("Heirarchical Z-Buffer Info");
        ImGui::Text("Vertex Count: %d", zbuffer->vertices.size());
        ImGui::Text("Face Count: %d", zbuffer->faces.size());
    }
};