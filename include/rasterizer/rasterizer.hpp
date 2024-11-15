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
    ~Rasterizer() {}
    void init(int width, int height);
    void run() { window->update(); };
    virtual void render() = 0; // main render function
    void loadOBJ(const std::string &filename)
    {
        scene->addOBJ(filename);
    }

    Canvas *getCanvas() { return canvas.get(); }
    Scene *getScene() { return scene.get(); }

private:
    int width, height;
    const std::string class_name = "Rasterizer";
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Canvas> canvas;
    std::unique_ptr<Window> window;
};

class EzRasterizer : public Rasterizer
{
public:
    EzRasterizer() {}
    ~EzRasterizer() {}
    void render() override
    {
        std::cout << "EzRasterizer render" << std::endl;
    };
};