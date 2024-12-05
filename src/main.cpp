#include <iostream>

// #include <rasterizer/rasterizer.hpp>
#include <zbuffer/scanline.hpp>
#include <utils.hpp>

int main()
{
    bool isGPU = true;
    // isGPU = false;
    // auto rasterizer = std::make_unique<EzRasterizer>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    auto rasterizer = std::make_unique<Scanline>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT, isGPU);
    rasterizer->loadOBJ("./assets/teapot.obj", "debug");
    rasterizer->addComputeShader("debug");

    if (isGPU)
        rasterizer->setCamera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // gpu version
    else
    {
        rasterizer->implementTransform("teapot.obj", SCRA::Utils::TranslateMatrix(0.0f, -1.5f, 0.0f));
        rasterizer->implementTransform("teapot.obj", SCRA::Utils::RotateMatrix(0.0f, 0.0f, 0.0f));
        rasterizer->implementTransform("teapot.obj", SCRA::Utils::ScaleMatrix(0.1f, 0.1f, 0.1f));
        rasterizer->setCamera(glm::vec3(100.0f, 100.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // cpu version
    }

    rasterizer->run();
}