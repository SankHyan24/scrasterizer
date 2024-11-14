#pragma once
#include <memory>
#include <glm/glm.hpp>

#include <rasterizer/scene.hpp>
#include <window/canvas.hpp>
#include <window/glwindow.hpp>

class Rasterizer
{
public:
    Rasterizer() {}
    ~Rasterizer() {}
    void init(int width, int height)
    {
        scene = std::make_unique<Scene>();
        canvas = std::make_unique<Canvas>(width, height);
        window = std::make_unique<Window>(width, height, class_name.c_str());
        window->bindTextureMap(canvas->getTextureMap());
        // RenderFunc screen = [](char *map, int w, int h)
        // {
        //     // ImGui::Begin("CharMatrixVisualization");
        //     // ImGui::End();
        // };
        // window->setRenderFunc(screen);
    }
    void run() { window->update(); };
    virtual void render() = 0; // main render function
    void loadOBJ(const std::string &filename)
    {
        scene->addOBJ(filename);
    }

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