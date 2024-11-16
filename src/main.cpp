#include <iostream>

#include <rasterizer/rasterizer.hpp>
#include <utils.hpp>

int main()
{
    auto rasterizer = std::make_unique<EzRasterizer>(SCRA::Config::WIDTH, SCRA::Config::HEIGHT);
    rasterizer->loadOBJ("../assets/teapot.obj", "basic");

    rasterizer->implementTransform("teapot.obj", SCRA::Utils::TranslateMatrix(0.0f, -1.5f, 0.0f));
    rasterizer->implementTransform("teapot.obj", SCRA::Utils::RotateMatrix(0.0f, 0.0f, 0.0f));
    rasterizer->implementTransform("teapot.obj", SCRA::Utils::ScaleMatrix(0.1f, 0.1f, 0.1f));

    rasterizer->setCamera(glm::vec3(100.0f, 100.0f, 100.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

    rasterizer->run();
}