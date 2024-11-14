#include <iostream>
#include <memory>
#include <window/glwindow.hpp>
#include <obj_loader/obj_loader.hpp>
#include <rasterizer/rasterizer.hpp>

const int HEIGHT = 800;
const int WIDTH = 800;
int main()
{
    auto rasterizer = std::make_unique<EzRasterizer>();
    rasterizer->init(WIDTH, HEIGHT);
    rasterizer->loadOBJ("../assets/teapot.obj");
    rasterizer->run();
}