#pragma once
#include <rasterizer/rasterizer.hpp>
class Scanline : public Rasterizer
{
public:
    Scanline() : Rasterizer() {}
    Scanline(int width, int height) : Rasterizer(width, height) {}

    ~Scanline() {}
    void render() override
    {
    }

    void renderGPU() override
    {
        autoRotateCamera();
        scene->drawGPU();
    }

private:
};