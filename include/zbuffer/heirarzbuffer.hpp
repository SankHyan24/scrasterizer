#pragma once
#include <zbuffer/naivezbuffer.hpp>
#include <core/bvh.hpp>

class HeirarZBuffer
{
public:
    int height, width;
    float *zBufferData{nullptr};
    float *colorBufferData{nullptr};
    Uint pixelPerTileX, pixelPerTileY;
    Uint tileCountX, tileCountY;
    float *maxZ;
    HeirarZBuffer(int width, int height, float *zBufferData, float *colorBufferData, Uint pixelPerTileX = 32, Uint pixelPerTileY = 32)
        : width(width), height(height), pixelPerTileX(pixelPerTileX), pixelPerTileY(pixelPerTileY), zBufferData(zBufferData), colorBufferData(colorBufferData)
    {
        tileCountX = width / pixelPerTileX;
        tileCountY = height / pixelPerTileY;
        if (width % pixelPerTileX != 0)
            tileCountX++;
        if (height % pixelPerTileY != 0)
            tileCountY++;
        maxZ = new float[tileCountX * tileCountY];
        std::fill(maxZ, maxZ + tileCountX * tileCountY, std::numeric_limits<float>::infinity());

        std::cout << "HeirarZBuffer construction done" << std::endl;
        std::cout << "Have " << tileCountX << " tiles in x direction, and " << tileCountY << " tiles in y direction" << std::endl;
    }
    ~HeirarZBuffer()
    {
        delete[] maxZ;
    }
    void resetMaxZ()
    {
        std::fill(maxZ, maxZ + tileCountX * tileCountY, std::numeric_limits<float>::infinity());
    }
    bool ifTileNeedRender(Sint2 bbpMin, Sint2 bbpMax, Uint idx, Uint idy, float bbMinZ)
    {
        assert(idx < tileCountX && idy < tileCountY && "ifTileNeedRender: idx or idy out of range");
        int pxMin = idx * pixelPerTileX, pxMax = (idx + 1) * pixelPerTileX;
        int pyMin = idy * pixelPerTileY, pyMax = (idy + 1) * pixelPerTileY;
        pxMax = pxMax >= width ? width - 1 : pxMax;
        pyMax = pyMax >= height ? height - 1 : pyMax;
        if (bbpMin.x > pxMax || bbpMax.x < pxMin || bbpMin.y > pyMax || bbpMax.y < pyMin)
            return false;
        if (bbMinZ < maxZ[idy * tileCountX + idx])
            return true;
        return false;
    }

    void updateTileMaxZ(Uint idx, Uint idy)
    {
        assert(idx < tileCountX && idy < tileCountY && "updateTileMaxZ: idx or idy out of range");
        float maxZ_ = -std::numeric_limits<float>::infinity();
        int xMin = idx * pixelPerTileX, xMax = (idx + 1) * pixelPerTileX;
        int yMin = idy * pixelPerTileY, yMax = (idy + 1) * pixelPerTileY;
        for (int i = yMin; i < yMax && i < height; i++)
            for (int j = xMin; j < xMax && j < width; j++)
                maxZ_ = maxZ_ < zBufferData[i * width + j] ? zBufferData[i * width + j] : maxZ_;
        maxZ[idy * tileCountX + idx] = maxZ_;
    }

    void drawZ()
    {
        for (int i = 0; i < tileCountY; i++)
            for (int j = 0; j < tileCountX; j++)
                __drawTileByZValue(j, i);
    }
    void __drawTileByZValue(Uint idx, Uint idy)
    {
        assert(idx < tileCountX && idy < tileCountY && "drawTileByZValue: idx or idy out of range");
        for (int i = idy * pixelPerTileY; i < (idy + 1) * pixelPerTileY; i++)
            for (int j = idx * pixelPerTileX; j < (idx + 1) * pixelPerTileX; j++)
            {
                if (maxZ[idy * tileCountX + idx] == std::numeric_limits<float>::infinity())
                    continue;
                colorBufferData[i * width * 3 + j * 3] = maxZ[idy * tileCountX + idx];
                colorBufferData[i * width * 3 + j * 3 + 1] = maxZ[idy * tileCountX + idx];
                colorBufferData[i * width * 3 + j * 3 + 2] = maxZ[idy * tileCountX + idx];
            }
    }
};
class HeirarZBufferHelper
{
public:
    int height, width;
    float *zBufferData{nullptr};
    float *colorBufferData{nullptr};
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::unique_ptr<RasterBVH> bvh{nullptr};
    std::unique_ptr<HeirarZBuffer> tileManager{nullptr};
    glm::mat4 mvp;
    Camera &camera;

    HeirarZBufferHelper(int width, int height, Camera &cam) : width(width), height(height), camera(cam)
    {
        zBufferData = new float[width * height];
        colorBufferData = new float[width * height * 3];
        tileManager = std::make_unique<HeirarZBuffer>(width, height, zBufferData, colorBufferData);
    }
    ~HeirarZBufferHelper()
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
        bvh = std::make_unique<RasterBVH>(faces, vertices, 0.01, 0.020);
        // set debug and render callback
        BVHDebugCallBack debug = [this](BoundingBox3f bb) -> bool
        {
            __drawBoundingBoxFrame(bb);
            return true;
        };
        bvh->_traversalDebug = debug;
        BVHRenderCallBack render = [this](BVHBuildNode *node, RasterBVHContext &context) -> bool
        {
            if (node->splitAxis != 3) // internal node
                return __drawBoundingBoxInter(*node, context);
            return __drawBoundingBoxLeaf(*node, context); // leaf node
        };
        bvh->_traversalRenderCallback = render;
    }
    void __fragmentShader()
    {
        tileManager->resetMaxZ();
        bvh->traversalBVH();
    }
    bool __drawBoundingBoxLeaf(BVHBuildNode &node, RasterBVHContext &context)
    {
        Uint needTileFromX{0}, needTileToX{tileManager->tileCountX}, needTileFromY{0}, needTileToY{tileManager->tileCountY};
        bool need_draw = false;
        std::vector<Uint> tileMaxZUpdateList;
        glm::mat4 mat = mvp;
        auto &bb = node.bounds;

        float newBBxMin = bb.pMinNew.x, newBBxMax = bb.pMaxNew.x;
        float newBByMin = bb.pMinNew.y, newBByMax = bb.pMaxNew.y;
        float bbminZ = bb.pMinNew.z;
        // to screen space
        newBBxMin = (newBBxMin + 1.0f) * width / 2.0f;
        newBBxMax = (newBBxMax + 1.0f) * width / 2.0f;
        newBByMin = (newBByMin + 1.0f) * height / 2.0f;
        newBByMax = (newBByMax + 1.0f) * height / 2.0f;
        Sint2 newBBMin = Sint2(std::floor(newBBxMin), std::floor(newBByMin));
        Sint2 newBBMax = Sint2(std::ceil(newBBxMax), std::ceil(newBByMax));
        for (int i = needTileFromX; i < needTileToX; i++)
            for (int j = needTileFromY; j < needTileToY; j++)
                if (tileManager->ifTileNeedRender(newBBMin, newBBMax, i, j, bbminZ))
                {
                    tileMaxZUpdateList.push_back(i);
                    tileMaxZUpdateList.push_back(j);
                    need_draw = true;
                }
        if (!need_draw)
            return false;
        std::vector<Uint> face_render_list;
        std::vector<Uint> &orderdata = bvh->orderedData;
        for (int i = node.firstPrimOffset; i < node.firstPrimOffset + node.nPrimitives; i++)
        {
            Uint faceIndex = orderdata[i];
            face_render_list.push_back(faceIndex);
        }
        for (int i = 0; i < face_render_list.size(); i++)
        {
            auto &face = faces[face_render_list[i]];
            __drawTriangle(face);
        }
        for (int i = 0; i < tileMaxZUpdateList.size(); i += 2)
        {
            tileManager->updateTileMaxZ(tileMaxZUpdateList[i], tileMaxZUpdateList[i + 1]);
        }
        context.culledFaces -= face_render_list.size();
        return true;
    }
    bool __drawBoundingBoxInter(BVHBuildNode &node, RasterBVHContext &context)
    {
        auto &bb = node.bounds;
        Uint needTileFromX{0}, needTileToX{tileManager->tileCountX}, needTileFromY{0}, needTileToY{tileManager->tileCountY}; // 需要绘制的范围
        bool need_draw = false;

        float newBBxMin = bb.pMinNew.x, newBBxMax = bb.pMaxNew.x;
        float newBByMin = bb.pMinNew.y, newBByMax = bb.pMaxNew.y;
        float bbminZ = bb.pMinNew.z;
        // to screen space
        newBBxMin = (newBBxMin + 1.0f) * width / 2.0f;
        newBBxMax = (newBBxMax + 1.0f) * width / 2.0f;
        newBByMin = (newBByMin + 1.0f) * height / 2.0f;
        newBByMax = (newBByMax + 1.0f) * height / 2.0f;
        Sint2 newBBMin = Sint2(std::floor(newBBxMin), std::floor(newBByMin));
        Sint2 newBBMax = Sint2(std::ceil(newBBxMax), std::ceil(newBByMax));
        for (int i = needTileFromX; i < needTileToX; i++)
        {
            for (int j = needTileFromY; j < needTileToY; j++)
            {
                if (tileManager->ifTileNeedRender(newBBMin, newBBMax, i, j, bbminZ))
                {
                    need_draw = true;
                    break;
                }
            }
            if (need_draw)
                break;
        }
        return need_draw;
    }
    void __drawBoundingBoxFrame(BoundingBox3f &bb)
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
    void __drawTriangle(const Face &face)
    {
        auto &v0_ = vertices[face.v0];
        auto &v1_ = vertices[face.v1];
        auto &v2_ = vertices[face.v2];
        auto v0Screen = glm::vec4(v0_.x, v0_.y, v0_.z, 1.0f);
        auto v1Screen = glm::vec4(v1_.x, v1_.y, v1_.z, 1.0f);
        auto v2Screen = glm::vec4(v2_.x, v2_.y, v2_.z, 1.0f);
        v0Screen = mvp * v0Screen;
        v1Screen = mvp * v1Screen;
        v2Screen = mvp * v2Screen;
        v0Screen /= v0Screen.w;
        v1Screen /= v1Screen.w;
        v2Screen /= v2Screen.w;
        // to screen space
        v0Screen.x = (v0Screen.x + 1.0f) * 0.5f * width;
        v0Screen.y = (1.0f + v0Screen.y) * 0.5f * height;
        v1Screen.x = (v1Screen.x + 1.0f) * 0.5f * width;
        v1Screen.y = (1.0f + v1Screen.y) * 0.5f * height;
        v2Screen.x = (v2Screen.x + 1.0f) * 0.5f * width;
        v2Screen.y = (1.0f + v2Screen.y) * 0.5f * height;
        int xMin = v0Screen.x < v1Screen.x ? v0Screen.x : v1Screen.x;
        xMin = xMin < v2Screen.x ? xMin : v2Screen.x;
        int xMax = v0Screen.x > v1Screen.x ? v0Screen.x : v1Screen.x;
        xMax = xMax > v2Screen.x ? xMax : v2Screen.x;
        int yMin = v0Screen.y < v1Screen.y ? v0Screen.y : v1Screen.y;
        yMin = yMin < v2Screen.y ? yMin : v2Screen.y;
        int yMax = v0Screen.y > v1Screen.y ? v0Screen.y : v1Screen.y;
        yMax = yMax > v2Screen.y ? yMax : v2Screen.y;
        // draw triangle
        Vertex tmpV0(v0Screen.x, v0Screen.y, v0Screen.z, v0_.nx, v0_.ny, v0_.nz);
        Vertex tmpV1(v1Screen.x, v1Screen.y, v1Screen.z, v1_.nx, v1_.ny, v1_.nz);
        Vertex tmpV2(v2Screen.x, v2Screen.y, v2Screen.z, v2_.nx, v2_.ny, v2_.nz);
        for (int i = xMin < 0 ? 0 : xMin; i <= xMax && i < width; i++)
            for (int j = yMin < 0 ? 0 : yMin; j <= yMax && j < height; j++)
            {
                float lambda1, lambda2, lambda3;
                __ComputeBarycentricCoords(i, j, tmpV0, tmpV1, tmpV2, lambda2, lambda3);
                lambda1 = 1 - lambda2 - lambda3;
                if (__ifLambda12InsideTriangle(lambda1, lambda2))
                {
                    // draw fragment
                    float z = lambda1 * tmpV0.z + lambda2 * tmpV1.z + (1 - lambda1 - lambda2) * tmpV2.z;
                    if (z <= zBufferData[j * width + i])
                    {
                        float r = lambda1 * v0_.nx + lambda2 * v1_.nx + (1 - lambda1 - lambda2) * v2_.nx;
                        float g = lambda1 * v0_.ny + lambda2 * v1_.ny + (1 - lambda1 - lambda2) * v2_.ny;
                        float b = lambda1 * v0_.nz + lambda2 * v1_.nz + (1 - lambda1 - lambda2) * v2_.nz;
                        zBufferData[j * width + i] = z;
                        colorBufferData[(j * width + i) * 3] = r;
                        colorBufferData[(j * width + i) * 3 + 1] = g;
                        colorBufferData[(j * width + i) * 3 + 2] = b;
                    }
                }
            }
    }
    void __ComputeBarycentricCoords(int x, int y, const Vertex &v0, const Vertex &v1, const Vertex &v2, float &lambda1, float &lambda2)
    {
        // barycentric coordinate
        float area = 0.5f * (-v1.y * v2.x + v0.y * (-v1.x + v2.x) + v0.x * (v1.y - v2.y) + v1.x * v2.y);
        lambda1 = 1 / (2 * area) * (v0.y * v2.x - v0.x * v2.y + (v2.y - v0.y) * x + (v0.x - v2.x) * y);
        lambda2 = 1 / (2 * area) * (v0.x * v1.y - v0.y * v1.x + (v0.y - v1.y) * x + (v1.x - v0.x) * y);
    }
    bool __ifLambda12InsideTriangle(float l1, float l2)
    {
        return l1 >= 0 && l2 >= 0 && 1 - l1 - l2 >= 0;
    }
    int obj_vertex_offset{0}; // for each obj, the offset of vertices
};

class HeirarZBufferRaster : public Rasterizer
{
public:
    HeirarZBufferRaster(int width, int height, bool isGPU = true) : Rasterizer(width, height, isGPU)
    {
        zbuffer = std::make_unique<HeirarZBufferHelper>(width, height, scene->getCameraV());
        zBufferPrecompute = zbuffer->zBufferData;
        colorPrecompute = zbuffer->colorBufferData;
        zbuffer->camera = scene->getCameraV();
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
    std::unique_ptr<HeirarZBufferHelper> zbuffer;
    float *colorPrecompute{nullptr};

    void __showHZBufferDataStructInfo()
    {
        _showimguiSubTitle("Heirarchical Z-Buffer Info");
        ImGui::Text("Split Method: %s", RasterBVH::SplitMethodToString(zbuffer->bvh->_method).c_str());
        ImGui::Text("Vertex Count: %d", zbuffer->vertices.size());
        ImGui::Text("Face Count: %d", zbuffer->faces.size());
        ImGui::Text("BVH Total Nodes: %d", zbuffer->bvh->_totalNodes);
        ImGui::Text("BVH Depth: %d", zbuffer->bvh->_maxDepth);
        ImGui::Separator();
        ImGui::Text("Culled Nodes: %8d  ratio(%2.1f%%)", zbuffer->bvh->_context.culledNodes, zbuffer->bvh->_context.culledNodes * 100.0f / zbuffer->bvh->_totalNodes);
        ImGui::Text("Culled Faces: %8d  ratio(%2.1f%%)", zbuffer->bvh->_context.culledFaces, zbuffer->bvh->_context.culledFaces * 100.0f / zbuffer->faces.size());
    }
};