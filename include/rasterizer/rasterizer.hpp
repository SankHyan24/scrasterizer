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

    // window init interface
    virtual int renderInit() = 0;
    // imgui interface
    virtual int renderImGui() { return 1; };
    // render interface
    virtual void render() = 0;    // main render function cpu
    virtual void renderGPU() = 0; // main render function opengl

    void loadOBJ(const std::string &filename);
    void loadOBJ(const std::string &filename, const std::string &shadername) { scene->addOBJ(filename, shadername); }
    void setCamera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
    {
        scene->getCameraV().updateCamera(position, target, up);
        camLengthToTarget = glm::length(position - target);
        camLengthToTargetOld = camLengthToTarget;
    }
    void implementTransform(std::string file_name, const glm::mat4 &transform);
    void implementTransform(int index, const glm::mat4 &transform) { scene->transformObj(index, transform); }

    // others
    void addComputeShader(const std::string &shadername) { scene->addComputeShader(shadername); }

protected:
    float *zBufferPrecompute{nullptr};

    float camAutoRotateSpeed{1.0f};
    void _autoRotateCamera();
    float camHoriTheta{0.25f};
    float camHoriThetaOld{0.25f};
    void _setCameraTheta();
    float camLengthToTarget{8.660254f};
    float camLengthToTargetOld{8.660254f};
    void _setCameraLengthToTarget();

    void _drawCoordinateAxis();
    void _drawLine(const glm::vec2 &p0, const glm::vec2 &p1, char r = 255, char g = 255, char b = 255);
    void _drawLineScreenSpace(const glm::vec2 &p0, const glm::vec2 &p1, char r = 255, char g = 255, char b = 255);
    void _drawTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2);

    void _showCameraInfoInImgui();
    void _showObjInfosInImgui();
    void _showimguiSubTitle(const std::string &title);

    // GPU
    bool isGPU{false};

    //
    int width, height;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Canvas> canvas;
    std::unique_ptr<Window> window;

private:
    virtual void __printHello();
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
        for (int i = 0; i < objs.size(); i++)
        {
            if (!scene->obj_activated[i])
                continue;
            auto &obj = objs[i];
            auto vertices = obj->getVertices();
            auto faces = obj->getFaces();
            for (auto &face : faces)
            {
                auto v0 = vertices[face.v0];
                auto v1 = vertices[face.v1];
                auto v2 = vertices[face.v2];
                auto p0 = glm::vec4(viewProjectionMatrix * glm::vec4(v0.x, v0.y, v0.z, 1.0f));
                p0 = p0 / p0.w;
                auto p1 = glm::vec4(viewProjectionMatrix * glm::vec4(v1.x, v1.y, v1.z, 1.0f));
                p1 = p1 / p1.w;
                auto p2 = glm::vec4(viewProjectionMatrix * glm::vec4(v2.x, v2.y, v2.z, 1.0f));
                p2 = p2 / p2.w;
                _drawTriangle(glm::vec3(p0), glm::vec3(p1), glm::vec3(p2));
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

    int renderImGui() override
    {
        _showCameraInfoInImgui();
        _showObjInfosInImgui();
        return 1;
    }

private:
};