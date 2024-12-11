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
        float xStart, xEnd, yMin; // xyz
        float zStart, zEnd, yMax;
        float xSlope, zSlope;
        float rStart, gStart, bStart; // rgb
        float rEnd, gEnd, bEnd;
        float rSlope, gSlope, bSlope;

        Edge(Vertex v0, Vertex v1, int eId);
        bool if_paired{false};
        int eId, pId;

        float xStartFixed, yStartFixed, zStartFixed;
        float rStartFixed, gStartFixed, bStartFixed;
        void edgeStepToIntegerY(int integerY);
    };

    struct Polygon
    {
        std::vector<Vertex> vertices;
        float a, b, c, d;
        int dy;
        int pid;
    };

    using EdgeTable = std::vector<Edge>;
    using EdgeIdTable = std::vector<int>;

    int obj_vertex_offset{0}; // for each obj, the offset of vertices
    int obj_face_offset{0};   // for each obj, the offset of vertices
    std::vector<Vertex> vertices;
    EdgeTable TotalEdgeTable;
    EdgeTable activeEdgeTable;
    std::vector<EdgeIdTable> activeTable;   // for each scanline, the edges to be added to the active edge table
    std::vector<EdgeIdTable> deactiveTable; // for each scanline, the edges to be deleted from the active edge table

    std::vector<float> zBuffer;
    float *zBufferData{nullptr};

    Scanline(int width, int height);

    void init();
    void buildTable(OBJ &obj, Camera &camera);
    void scanScreen();

    // for debug
    float avgEdgeCount{0.0f};

private:
    int edgeIdCounter{0};
    void __buildActivateDeactivateTable(Polygon &polygon);
    int __findNextSamePolyEdge(int pId);
};

class ScanlineRaster : public Rasterizer
{
public:
    ScanlineRaster(int width, int height, bool isGPU = true);
    ~ScanlineRaster();

    void render() override;
    void renderGPU() override;
    int renderInit() override;
    int renderImGui() override
    {
        _showCameraInfoInImgui();
        _showObjInfosInImgui();
        __showScanlineDataStructInfo();
        return 1;
    }

private:
    void __ScanLinePreInit();
    void __ScanLinePreCompute();
    void __ScanLinePreDebug();
    GLuint ssbo;
    std::unique_ptr<Scanline> scanline;
    int debug_count{0};

    void __showScanlineDataStructInfo();
};
