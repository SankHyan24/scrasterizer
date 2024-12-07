#pragma once
#include <rasterizer/rasterizer.hpp>
#include <cassert>
#include <fstream>

class Scanline
{
public:
    int height, width;
    struct Edge
    {
        int eId;              // 边的唯一标识符
        int pId;              // 边所属的多边形的 id
        int addScanlineId;    // 边将要被添加到的扫描线的 id
        int deleteScanlineId; // 边将要被删除的扫描线的 id
        float xStart, xEnd;   // 边的起始和结束 x 坐标
        float zStart, zEnd;   // 边的起始和结束深度值
        float yMin, yMax;     // 边的扫描线范围
        float xSlope;         // 边的 x 增量 (dx/dy)
        float zSlope;         // 深度的增量 (dz/dy)
        Edge(Vertex v0, Vertex v1, int eId) : eId(eId)
        {
            xStart = v0.x;
            xEnd = v1.x;
            zStart = v0.z;
            zEnd = v1.z;
            yMin = v0.y;
            yMax = v1.y;
            xSlope = (xEnd - xStart) / (yMax - yMin);
            zSlope = (zEnd - zStart) / (yMax - yMin);
        }
        bool if_paired{false};
    };

    struct Polygon
    {
        std::vector<Vertex> vertices; // 多边形的顶点
        float a, b, c, d;
        int dy; // 多边形跨越的扫描线数目
        int id; // 唯一标识符，用于深度比较
    };

    using EdgeTable = std::vector<Edge>;
    using EdgeIdTable = std::vector<int>;

    EdgeTable TotalEdgeTable;
    EdgeTable activeEdgeTable;
    std::vector<EdgeIdTable> activeTable;   // for each scanline, the edges to be added to the active edge table
    std::vector<EdgeIdTable> deactiveTable; // for each scanline, the edges to be deleted from the active edge table

    std::vector<float> zBuffer;
    float *zBufferData{nullptr};

    Scanline(int width, int height) : width(width), height(height)
    {
        zBufferData = new float[width * height];
    }

    Scanline() = delete;

    void buildTable(OBJ &obj, Camera &camera)
    {
        activeTable.clear();
        activeTable.resize(height);
        deactiveTable.clear();
        deactiveTable.resize(height);
        zBuffer.resize(width * height);
        std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());
        std::fill(zBufferData, zBufferData + width * height, 0);
        auto &faces = obj.getFaces();
        auto &vertices_ = obj.getVertices();
        auto vertices = vertices_;
        glm::mat4 MVP = camera.getViewProjectionMatrix();
        for (auto &v : vertices)
        {
            glm::vec4 v4(v.x, v.y, v.z, 1.0f);
            v4 = MVP * v4;
            v.x = v4.x / v4.w;
            v.y = v4.y / v4.w;
            v.z = v4.z / v4.w;
            // from world space to screen space
            v.x = (v.x + 1.0f) * width / 2.0f;
            v.y = (v.y + 1.0f) * height / 2.0f;
            // // use round
            // int x = std::round(v.x);
            // int y = std::round(v.y);
            // v.x = x;
            // v.y = y;
            // zBufferData[y * width + x] = 1;
        }

        for (int face_index = 0; face_index < faces.size(); face_index++)
        {
            auto &face = faces[face_index];
            Polygon polygon;
            polygon.id = face_index;
            polygon.vertices.push_back(vertices[face.v0]);
            polygon.vertices.push_back(vertices[face.v1]);
            polygon.vertices.push_back(vertices[face.v2]);
            // compute the plane equation
            auto &v0 = polygon.vertices[0];
            auto &v1 = polygon.vertices[1];
            auto &v2 = polygon.vertices[2];
            polygon.a = (v1.y - v0.y) * (v2.z - v0.z) - (v2.y - v0.y) * (v1.z - v0.z);
            polygon.b = (v1.z - v0.z) * (v2.x - v0.x) - (v2.z - v0.z) * (v1.x - v0.x);
            polygon.c = (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
            polygon.d = -(polygon.a * v0.x + polygon.b * v0.y + polygon.c * v0.z);
            if (polygon.c == 0)
                continue;
            __buildActivateDeactivateTable(polygon);
        }
    }

    void scanScreen()
    {
        for (int y = 0; y < height; y++)
        {
            int num = activeEdgeTable.size();
            for (auto &eId : activeTable[y])
                activeEdgeTable.push_back(TotalEdgeTable[eId]);
            int tobeAdded = activeTable[y].size();
            for (auto &eId : deactiveTable[y])
            {
                bool found = false;
                for (int i = 0; i < activeEdgeTable.size(); i++)
                {
                    if (activeEdgeTable[i].eId == eId)
                    {
                        activeEdgeTable.erase(activeEdgeTable.begin() + i);
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    std::string message = "edge [" + std::to_string(eId) + "] not found";
                    assert(false && "edge not found");
                }
            }
            int tobeDeleted = deactiveTable[y].size();
            assert(num + tobeAdded - tobeDeleted == activeEdgeTable.size());

            // sort the active edge table
            std::sort(activeEdgeTable.begin(), activeEdgeTable.end(), [](const Edge &e0, const Edge &e1) -> bool
                      { return e0.xStart < e1.xStart; });

            // if (activeEdgeTable.size() % 2 != 0)
            // {
            //     std::cout << "y = " << y << ", tobeAdded = " << tobeAdded << ", tobeDeleted = " << tobeDeleted << std::endl;
            //     std::cout << "  edge number before = " << num << std::endl;
            //     std::cout << "  expected active edge table size = " << num + tobeAdded - tobeDeleted << std::endl;
            //     std::cout << "  actual active edge table size = " << activeEdgeTable.size() << std::endl;
            //     std::cout << "active edge table size = " << activeEdgeTable.size() << std::endl;
            //     std::cout << "Active Edge Table: " << std::endl;
            //     for (auto &e : activeEdgeTable)
            //         std::cout << "  eId = " << e.eId << "  addScanlineId = " << e.addScanlineId << " pId = " << e.pId << std::endl;
            //     std::cout << "Edges added: " << std::endl;
            //     for (auto &eId : activeTable[y])
            //         std::cout << "  eId = " << eId << std::endl;
            //     std::cout << "Edges deleted: " << std::endl;
            //     for (auto &eId : deactiveTable[y])
            //         std::cout << "  eId = " << eId << std::endl;

            std::vector<std::pair<int, int>> PID_counts;
            for (auto &edge : activeEdgeTable)
            {
                bool found = false;
                for (auto &pair : PID_counts)
                {
                    if (pair.first == edge.pId)
                    {
                        pair.second++;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    PID_counts.push_back(std::make_pair(edge.pId, 1));
            }
            for (auto &pair : PID_counts)
                assert(pair.second == 2 && "active edge table size must be even");

            for (int i = 0; i < activeEdgeTable.size(); i++)
            {
                auto &edge0 = activeEdgeTable[i];
                if (edge0.if_paired)
                    continue;
                edge0.if_paired = true;
                auto &edge1 = activeEdgeTable[__findNextSamePolyEdge(edge0.pId)];
                int xStart = std::ceil(edge0.xStart);
                int xEnd = std::ceil(edge1.xStart);
                if (xStart == xEnd)
                    continue;
                float zStart = edge0.zStart;
                float zEnd = edge1.zStart;
                float zSlope = (zEnd - zStart) / (xEnd - xStart);
                for (int x = xStart > 0 ? xStart : 0; x < xEnd && x < width; x++)
                {
                    if (x < 0 || x >= width)
                        continue;
                    if (zBuffer[y * width + x] > zStart)
                    {
                        zBuffer[y * width + x] = zStart;
                        zBufferData[y * width + x] = zStart;
                    }
                    zStart += zSlope;
                }
            }
            for (auto &e : activeEdgeTable)
            {
                e.xStart += e.xSlope;
                e.zStart += e.zSlope;
                e.if_paired = false;
            }
        }
    }

private:
    int edgeIdCounter{0};
    void __buildActivateDeactivateTable(Polygon &polygon)
    {
        for (int i = 0; i < polygon.vertices.size(); i++) // for each edge
        {
            auto &v0_ = polygon.vertices[i];
            auto &v1_ = polygon.vertices[(i + 1) % polygon.vertices.size()];

            auto v0 = v0_;
            auto v1 = v1_;

            // if the edge is horizontal, ignore it
            if (std::fabs(v0.y - v1.y) < SCRA::Config::EPSILON)
                continue;
            // if the edge is not intersecting with the scanline, ignore it
            if (std::ceil(v0.y) == std::ceil(v1.y))
                continue;
            if (v0.y > v1.y)
                std::swap(v0, v1);
            float y_ac_f = v0.y;
            float y_de_f = v1.y;
            if (y_ac_f >= height || y_de_f < 0)
                continue;
            int y_ac = std::ceil(y_ac_f < 0.0 ? 0.0 : y_ac_f);
            int y_de = std::ceil(y_de_f >= height ? height - 1 : y_de_f);

            Edge edge(v0, v1, edgeIdCounter++);
            activeTable[y_ac].push_back(edge.eId);
            edge.addScanlineId = y_ac;
            deactiveTable[y_de].push_back(edge.eId);
            edge.deleteScanlineId = y_de;
            edge.pId = polygon.id;
            // calculate the edge's xStart and xEnd, zStart and zEnd
            if (v0.y < 0)
            {
                edge.xStart = v0.x + (0 - v0.y) * edge.xSlope;
                edge.zStart = v0.z + (0 - v0.y) * edge.zSlope;
            }
            if (v1.y >= height)
            {
                edge.xEnd = v1.x - (v1.y - height) * edge.xSlope;
                edge.zEnd = v1.z - (v1.y - height) * edge.zSlope;
            }
            TotalEdgeTable.push_back(edge);
        }
    }

    int __findNextSamePolyEdge(int pId)
    {
        for (int i = 0; i < activeEdgeTable.size(); i++)
        {
            if (activeEdgeTable[i].pId == pId && activeEdgeTable[i].if_paired == false)
            {
                activeEdgeTable[i].if_paired = true;
                return i;
            }
        }
        assert(false && "edge not paired correctly");
        return -1;
    }
};

class ScanlineRaster : public Rasterizer
{
public:
    ScanlineRaster(int width, int height, bool isGPU = true) : Rasterizer(width, height, isGPU)
    {
        scanline = std::make_unique<Scanline>(width, height);
        zBufferPrecompute = scanline->zBufferData;
    }

    ~ScanlineRaster()
    {
        delete[] zBufferPrecompute;
    }
    void render() override { assert(false); /*not implemented*/ }

    void renderGPU() override
    {
        _autoRotateCamera();
        {
            auto program = scene->getComputeProgram("debug");
            program.use();
            ScanLinePreCompute();
            glDispatchCompute(this->width / SCRA::Config::CS_LOCAL_SIZE_X, this->height / SCRA::Config::CS_LOCAL_SIZE_Y, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        scene->drawGPU();
    }
    int renderInit() override
    {
        // disable depth test
        glDisable(GL_DEPTH_TEST);

        // use compute program to render
        scene->createTexture("imgOutput");

        // bind texture to shader
        {
            auto program = scene->getComputeProgram("debug");
            program.use();
            scene->bindImageTexture("imgOutput", 0);
            ScanLinePreInit();
        }
        return 1;
    }

private:
    void ScanLinePreInit()
    {
        glGenBuffers(1, &ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, width * height * sizeof(float), zBufferPrecompute, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    void ScanLinePreCompute()
    {
        scanline->buildTable(*scene->objs[0], scene->getCameraV());
        scanline->scanScreen();

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, width * height * sizeof(float), zBufferPrecompute);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        {
            static int count = 0;
            count++;
            if (count != 1)
                return;
            auto color = new char[width * height * 3];
            for (int i = 0; i < width * height; i++)
            {
                color[i * 3] = zBufferPrecompute[i] * 10;
                color[i * 3 + 1] = zBufferPrecompute[i] * 10;
                color[i * 3 + 2] = zBufferPrecompute[i] * 10;
            }
            SCRA::Utils::saveAsText(zBufferPrecompute, width, height, "zbuffer.txt");
            SCRA::Utils::saveAsPPM(color, width, height, "zbuffer.ppm");
        }
    }
    GLuint ssbo;
    float *zBufferPrecompute{nullptr};
    std::unique_ptr<Scanline> scanline;
};
