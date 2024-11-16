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
    Rasterizer(int width, int height, bool isGPU = true) : isGPU(isGPU)
    {
        init(width, height);
    }
    ~Rasterizer() {}

    void init(int width, int height);
    void run()
    {
        if (isGPU)
            scene->bindGPU();
        window->run();
    };
    virtual void render() = 0;    // main render function
    virtual void renderGPU() = 0; // main render function

    void loadOBJ(const std::string &filename, const std::string &shadername) { scene->addOBJ(filename); }
    void setCamera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
    {
        scene->getCameraV().updateCamera(position, target, up);
    }
    void implementTransform(std::string file_name, const glm::mat4 &transform)
    {
        int index = -1;
        for (int i = 0; i < scene->obj_filenames.size(); i++)
            if (scene->obj_filenames[i] == file_name)
            {
                index = i;
                break;
            }
        if (index == -1)
        {
            std::cerr << "Rasterizer::implementTransform file_name not found" << std::endl;
            return;
        }
        scene->transformObj(index, transform);
    }
    void implementTransform(int index, const glm::mat4 &transform)
    {
        scene->transformObj(index, transform);
    }

protected:
    void autoRotateCamera(float v = 1.0f)
    {
        // rotate camera around target
        float Dx = scene->getCameraV().getPosition().x - scene->getCameraV().getTarget().x;
        float Dz = scene->getCameraV().getPosition().z - scene->getCameraV().getTarget().z;
        float radius = sqrt(Dx * Dx + Dz * Dz);
        float angle = atan2(Dz, Dx);
        angle += glm::radians(v);
        float dx = radius * cos(angle);
        float dz = radius * sin(angle);
        auto &camera = scene->getCameraV();
        camera.movePosition(glm::vec3(dx, camera.getPosition().y, dz) - camera.getPosition());
    }
    void drawCoordinateAxis()
    {
        // 绘制3d坐标轴，只绘制正半轴
        auto &camera = scene->getCameraV();
        auto position = camera.getPosition();
        auto target = camera.getTarget();
        auto viewProjectionMatrix = camera.getViewProjectionMatrix();
        // x axis
        auto p0 = glm::vec3(0.0f, 0.0f, 0.0f);
        auto p1 = glm::vec3(100.0f, 0.0f, 0.0f);
        auto p0x = glm::vec3(viewProjectionMatrix * glm::vec4(p0.x, p0.y, p0.z, 1.0f));
        auto p1x = glm::vec3(viewProjectionMatrix * glm::vec4(p1.x, p1.y, p1.z, 1.0f));
        drawLine(glm::vec2(p0x.x, p0x.y), glm::vec2(p1x.x, p1x.y), 255, 0, 0);
        // y axis
        auto p2 = glm::vec3(0.0f, 0.0f, 0.0f);
        auto p3 = glm::vec3(0.0f, 100.0f, 0.0f);
        auto p2y = glm::vec3(viewProjectionMatrix * glm::vec4(p2.x, p2.y, p2.z, 1.0f));
        auto p3y = glm::vec3(viewProjectionMatrix * glm::vec4(p3.x, p3.y, p3.z, 1.0f));
        drawLine(glm::vec2(p2y.x, p2y.y), glm::vec2(p3y.x, p3y.y), 0, 255, 0);
        // z axis
        auto p4 = glm::vec3(0.0f, 0.0f, 0.0f);
        auto p5 = glm::vec3(0.0f, 0.0f, 100.0f);
        auto p4z = glm::vec3(viewProjectionMatrix * glm::vec4(p4.x, p4.y, p4.z, 1.0f));
        auto p5z = glm::vec3(viewProjectionMatrix * glm::vec4(p5.x, p5.y, p5.z, 1.0f));
        drawLine(glm::vec2(p4z.x, p4z.y), glm::vec2(p5z.x, p5z.y), 0, 0, 255);
    }
    void drawLine(const glm::vec2 &p0, const glm::vec2 &p1, char r = 255, char g = 255, char b = 255)
    {
        auto textureMap = canvas->getTextureMap();
        int width = canvas->getWidth();
        int height = canvas->getHeight();

        // from screen space[-1,1] to pixel space
        auto p0x = (p0.x + 1.0f) * 0.5f * width;
        auto p0y = (p0.y + 1.0f) * 0.5f * height;
        auto p1x = (p1.x + 1.0f) * 0.5f * width;
        auto p1y = (p1.y + 1.0f) * 0.5f * height;

        // Bresenham's line algorithm
        int x0 = p0x, y0 = p0y;
        int x1 = p1x, y1 = p1y;
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
    void drawTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
    {
        drawLine(glm::vec2(v0.x, v0.y), glm::vec2(v1.x, v1.y));
        drawLine(glm::vec2(v1.x, v1.y), glm::vec2(v2.x, v2.y));
        drawLine(glm::vec2(v2.x, v2.y), glm::vec2(v0.x, v0.y));
    }

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
    EzRasterizer(int width, int height) : Rasterizer(width, height) {}

    // ~EzRasterizer() {}
    void render() override
    {
        auto &objs = scene->objs;
        autoRotateCamera();
        drawCoordinateAxis();
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
                drawTriangle(p0, p1, p2);
            }
        }
    }

    void renderGPU() override
    {
        // use opengl to render
        // set camera
        scene->drawGPU();
    }

private:
};