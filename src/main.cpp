#include <iostream>

// #include <rasterizer/rasterizer.hpp>
#include <zbuffer/scanline.hpp>
#include <utils.hpp>

int main()
{
    bool isGPU = true;
    // isGPU = false;
    // auto rasterizer = std::make_unique<EzRasterizer>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    auto rasterizer = std::make_unique<ScanlineRaster>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    rasterizer->setCamera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    rasterizer->loadOBJ("./assets/bunny.obj", "scanline");
    rasterizer->implementTransform("bunny.obj", SCRA::Utils::TranslateMatrix(0.0f, -0.1f, 0.0f));
    rasterizer->implementTransform("bunny.obj", SCRA::Utils::ScaleMatrix(15.0f, 15.0f, 15.0f));

    // rasterizer->loadOBJ("./assets/teapot.obj", "scanline");
    // rasterizer->implementTransform("teapot.obj", SCRA::Utils::TranslateMatrix(0.0f, -1.5f, 0.0f));
    // rasterizer->implementTransform("teapot.obj", SCRA::Utils::ScaleMatrix(0.6f, 0.6f, 0.6f));

    rasterizer->run();
}