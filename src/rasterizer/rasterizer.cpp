#include <rasterizer/rasterizer.hpp>

void Rasterizer::init(int width, int height)
{
    // create scene, canvas, window
    scene = std::make_unique<Scene>(); // obtain OBJs and camera
    canvas = std::make_unique<Canvas>(width, height);
    window = std::make_unique<Window>(width, height, class_name.c_str());
    window->bindTextureMap(canvas->getTextureMap());
    RenderFunc screen = [this](void *r, int w, int h)
    {
        int a = 0;
        this->render();
        return a;
    };
    window->setRenderFunc(screen);
}