#include <iostream>

// #include <rasterizer/rasterizer.hpp>
#include <zbuffer/scanline.hpp>
#include <zbuffer/naivezbuffer.hpp>
#include <zbuffer/heirarzbuffer.hpp>
#include <utils.hpp>

#include <core/mipmap.hpp>
int main()
{
    bool isGPU = true;
    isGPU = false;
    // auto rasterizer = std::make_unique<EzRasterizer>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    // auto rasterizer = std::make_unique<ScanlineRaster>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    auto rasterizer = std::make_unique<NaiveZBufferRaster>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    // auto rasterizer = std::make_unique<HeirarZBufferRaster>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    rasterizer->setCamera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    rasterizer->loadOBJ("./assets/bunny.obj", "basic");
    // rasterizer->loadOBJ("./assets/bunny1.obj", "basic");
    // rasterizer->loadOBJ("./assets/bunnyL.obj", "basic");
    // rasterizer->loadOBJ("./assets/bunny14k.obj", "basic");
    // rasterizer->loadOBJ("./assets/complex_sphere.obj", "basic");
    rasterizer->implementTransform("bunny.obj", SCRA::Utils::TranslateMatrix(0.0f, -0.1f, 0.0f));
    // rasterizer->implementTransform("bunny1.obj", SCRA::Utils::TranslateMatrix(0.0f, 0.0f, 0.0f));
    rasterizer->implementTransform("bunny.obj", SCRA::Utils::ScaleMatrix(20.0f, 20.0f, 20.0f));
    // rasterizer->implementTransform("bunny1.obj", SCRA::Utils::ScaleMatrix(20.0f, 20.0f, 20.0f));

    rasterizer->loadOBJ("./assets/teapot.obj", "basic");
    rasterizer->implementTransform("teapot.obj", SCRA::Utils::TranslateMatrix(0.0f, -1.5f, 0.0f));
    rasterizer->implementTransform("teapot.obj", SCRA::Utils::ScaleMatrix(0.6f, 0.6f, 0.6f));

    rasterizer->run();
    std::cout << "Bye" << std::endl;
}