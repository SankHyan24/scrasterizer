#include <zbuffer/scanline.hpp>
Scanline::Edge::Edge(Vertex v0, Vertex v1, int eId) : eId(eId)
{
    xStart = v0.x; // xyz
    xEnd = v1.x;
    zStart = v0.z;
    zEnd = v1.z;
    yMin = v0.y;
    yMax = v1.y;
    xSlope = (xEnd - xStart) / (yMax - yMin);
    zSlope = (zEnd - zStart) / (yMax - yMin);
    rStart = v0.nx; // rgb
    gStart = v0.ny;
    bStart = v0.nz;
    rEnd = v1.nx;
    gEnd = v1.ny;
    bEnd = v1.nz;
    rSlope = (rEnd - rStart) / (yMax - yMin);
    gSlope = (gEnd - gStart) / (yMax - yMin);
    bSlope = (bEnd - bStart) / (yMax - yMin);
}

Scanline::Scanline(int width, int height) : width(width), height(height)
{
    zBufferData = new float[width * height];
    zBuffer.resize(width * height);
}

void Scanline::init()
{
    activeTable.clear();
    deactiveTable.clear();
    activeTable.resize(height);
    deactiveTable.resize(height);
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());
    std::fill(zBufferData, zBufferData + width * height, 0);
}

void Scanline::buildTable(OBJ &obj, Camera &camera)
{
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
        v.x = (v.x + 1.0f) * width / 2.0f; // from world space to screen space
        v.y = (v.y + 1.0f) * height / 2.0f;
    }

    for (int face_index = 0; face_index < faces.size(); face_index++)
    {
        auto &face = faces[face_index];
        Polygon polygon;
        polygon.pid = face_index;
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

void Scanline::scanScreen()
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
                assert(false && "edge not found");
        }
        int tobeDeleted = deactiveTable[y].size();
        assert(num + tobeAdded - tobeDeleted == activeEdgeTable.size() && "active edge table size not match");

        // sort the active edge table
        std::sort(activeEdgeTable.begin(), activeEdgeTable.end(), [](const Edge &e0, const Edge &e1) -> bool
                  { return e0.xStart < e1.xStart; });
        assert(activeEdgeTable.size() % 2 == 0 && "active edge table size not even");

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
            float zStart = edge0.zStart; // z buffer
            float zEnd = edge1.zStart;
            float zSlope = (zEnd - zStart) / (xEnd - xStart);
            float rStart = edge0.rStart; // rgb
            float rEnd = edge1.rStart;
            float rSlope = (rEnd - rStart) / (xEnd - xStart);
            float gStart = edge0.gStart;
            float gEnd = edge1.gStart;
            float gSlope = (gEnd - gStart) / (xEnd - xStart);
            float bStart = edge0.bStart;
            float bEnd = edge1.bStart;
            float bSlope = (bEnd - bStart) / (xEnd - xStart);
            for (int x = xStart > 0 ? xStart : 0; x < xEnd && x < width; x++) // fill the pixels
            {
                if (x < 0 || x >= width)
                    continue;
                if (zBuffer[y * width + x] > zStart) // depth test
                {
                    zBuffer[y * width + x] = zStart;
                    // normalize the color
                    float dist = std::sqrt(rStart * rStart + gStart * gStart + bStart * bStart);
                    int r_ = std::round(rStart / dist * 255);
                    int g_ = std::round(gStart / dist * 255);
                    int b_ = std::round(bStart / dist * 255);
                    Uchar rc = r_ > 255 ? 255 : (r_ < 0 ? 0 : r_);
                    Uchar gc = g_ > 255 ? 255 : (g_ < 0 ? 0 : g_);
                    Uchar bc = b_ > 255 ? 255 : (b_ < 0 ? 0 : b_);
                    Uchar ac = 255;
                    float value = SCRA::Utils::encodeCharsToFloat(rc, gc, bc, ac);
                    zBufferData[y * width + x] = value;
                }
                zStart += zSlope;
                rStart += rSlope;
                gStart += gSlope;
                bStart += bSlope;
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

void Scanline::__buildActivateDeactivateTable(Polygon &polygon)
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
        deactiveTable[y_de].push_back(edge.eId);
        edge.pId = polygon.pid;
        TotalEdgeTable.push_back(edge);
    }
}

int Scanline::__findNextSamePolyEdge(int pId)
{
    for (int i = 0; i < activeEdgeTable.size(); i++)
        if (activeEdgeTable[i].pId == pId && activeEdgeTable[i].if_paired == false)
        {
            activeEdgeTable[i].if_paired = true;
            return i;
        }
    assert(false && "edge not paired correctly");
    return -1;
}
