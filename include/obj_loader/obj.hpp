#pragma once
#include <vector>
#include <string>
#include <iostream>

#include <obj_loader/core.hpp>

class OBJ
{
public:
    OBJ() {}
    ~OBJ() {}
    void setFileName(const std::string &name) { file_name = name; }
    void addVertex(const Vertex &v) { vertices.push_back(v); }
    void addNormal(const Normal &n) { normals.push_back(n); }
    void addUV(const TexutreUV &uv) { uvs.push_back(uv); }
    void addFace(const Face &f) { faces.push_back(f); }

    std::string getFileName() const { return file_name; }
    const std::vector<Vertex> &getVertices() const { return vertices; }
    const std::vector<Normal> &getNormals() const { return normals; }
    const std::vector<TexutreUV> &getUVs() const { return uvs; }
    const std::vector<Face> &getFaces() const { return faces; }

    void printSummary() const
    {
        std::cout << "OBJ file [" << file_name << "] summary:" << std::endl;
        std::cout << "Vertices: " << vertices.size() << std::endl;
        std::cout << "Normals:  " << normals.size() << std::endl;
        std::cout << "UVs:      " << uvs.size() << std::endl;
        std::cout << "Faces:    " << faces.size() << std::endl;
    }

private:
    std::string file_name;
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<TexutreUV> uvs;
    std::vector<Face> faces;
};