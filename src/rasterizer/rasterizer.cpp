#include <rasterizer/rasterizer.hpp>

void Rasterizer::init(int width, int height)
{
    // create scene, canvas, window
    scene = std::make_unique<Scene>(); // to obtain OBJs and camera
    canvas = std::make_unique<Canvas>(width, height);
    window = std::make_unique<Window>(width, height, class_name.c_str());
    window->init(); // opengl, imgui setup
    if (isGPU)
        scene->loadShaders(); // compile shaders
    else
        window->bindTextureMap(canvas->getTextureMap());
    // CPU MODE
    RenderFunc screen = [this]() -> bool
    {
        if (this->isGPU)
        {
            this->renderGPU();
        }
        else
        {
            canvas->clearTextureMap();
            this->render();
        }
        return this->isGPU;
    };
    window->setRenderFunc(screen);
    // GPU MODE
    RenderFunc gpu = [this]() -> bool
    { return this->isGPU; };
    window->setRenderFuncGPU(gpu);
}