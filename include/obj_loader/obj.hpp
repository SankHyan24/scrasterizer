#pragma once
#include <vector>
#include <string>
#include <iostream>

#include <obj_loader/objtype.hpp>
#include <GL/gl3w.h>
#include <glm/glm.hpp>
class OBJ
{
public:
    OBJ() { vertices.push_back(Vertex(0.0f, 0.0f, 0.0f)); }
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

    void implementTransform(const glm::mat4 &transform)
    {
        for (auto &v : vertices)
        {
            glm::vec4 v4(v.x, v.y, v.z, 1.0f);
            v4 = transform * v4;
            v.x = v4.x;
            v.y = v4.y;
            v.z = v4.z;
        }
    }

    void bindGPU()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // Uint *data = new Uint[faces.size() * 3];

        auto data = std::unique_ptr<Uint[]>(new Uint[faces.size() * 3]);
        for (int i = 0; i < faces.size(); i++)
        {
            data[i * 3] = faces[i].v0;
            data[i * 3 + 1] = faces[i].v1;
            data[i * 3 + 2] = faces[i].v2;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Uint) * 3, data.get(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void drawGPU()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
    }

    void setModelMatrix(const glm::mat4 &t) { transform = t; }
    const glm::mat4 &getModelMatrix() const { return transform; }

private:
    std::string file_name;
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<TexutreUV> uvs;
    std::vector<Face> faces;

    // gpu
    unsigned int VBO, VAO, EBO;
    bool active{true};
    glm::mat4 transform{1.0f};
};