#pragma once
#include <memory>
#include <glm/glm.hpp>

#include <window/glwindow.hpp>
#include <rasterizer/scene.hpp>
#include <window/canvas.hpp>

class Rasterizer
{
public:
    Rasterizer() {}
    Rasterizer(int width, int height, bool isGPU = true) : isGPU(isGPU) { init(width, height); }
    ~Rasterizer() {}

    void init(int width, int height);
    void run();
    virtual void render() = 0;    // main render function cpu
    virtual void renderGPU() = 0; // main render function opengl

    virtual int renderInit() = 0;

    void loadOBJ(const std::string &filename, const std::string &shadername) { scene->addOBJ(filename, shadername); }
    void setCamera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up) { scene->getCameraV().updateCamera(position, target, up); }
    void implementTransform(std::string file_name, const glm::mat4 &transform);
    void implementTransform(int index, const glm::mat4 &transform) { scene->transformObj(index, transform); }

protected:
    void _autoRotateCamera(float v = 1.0f);
    void _drawCoordinateAxis();
    void _drawLine(const glm::vec2 &p0, const glm::vec2 &p1, char r = 255, char g = 255, char b = 255);
    void _drawTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2);

    // GPU
    bool isGPU{false};

    //
    int width, height;
    const std::string class_name = "Rasterizer";
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Canvas> canvas;
    std::unique_ptr<Window> window;
};

class EzRasterizer : public Rasterizer
{
public:
    EzRasterizer() : Rasterizer() {}
    EzRasterizer(int width, int height, bool isGPU = false) : Rasterizer(width, height, isGPU) {}

    ~EzRasterizer() {}
    void render() override
    {
        auto &objs = scene->objs;
        _autoRotateCamera();
        _drawCoordinateAxis();
        auto &camera = scene->getCameraV();
        auto viewProjectionMatrix = camera.getViewProjectionMatrix();
        for (auto &obj : objs)
        {
            auto vertices = obj->getVertices();
            auto faces = obj->getFaces();
            for (auto &face : faces)
            {
                auto v0 = vertices[face.v0];
                auto v1 = vertices[face.v1];
                auto v2 = vertices[face.v2];
                auto p0 = glm::vec3(viewProjectionMatrix * glm::vec4(v0.x, v0.y, v0.z, 1.0f));
                auto p1 = glm::vec3(viewProjectionMatrix * glm::vec4(v1.x, v1.y, v1.z, 1.0f));
                auto p2 = glm::vec3(viewProjectionMatrix * glm::vec4(v2.x, v2.y, v2.z, 1.0f));
                _drawTriangle(p0, p1, p2);
            }
        }
    }

    void renderGPU() override
    {
        _autoRotateCamera();
        scene->drawGPU();
    }

    int renderInit() override
    {
        return 1;
    }

private:
};