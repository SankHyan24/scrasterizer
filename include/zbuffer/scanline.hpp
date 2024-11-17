#pragma once
#include <rasterizer/rasterizer.hpp>
class Scanline : public Rasterizer
{
public:
    Scanline() : Rasterizer() {}
    Scanline(int width, int height, bool isGPU = true) : Rasterizer(width, height, isGPU) {}

    ~Scanline() {}
    void render() override {}

    void renderGPU() override
    {
        _autoRotateCamera();
        scene->drawGPU();
    }
    int renderInit() override
    {
        glDisable(GL_DEPTH_TEST);
        scene->createTexture("depthBuffer");
        return 1;
    }

private:
};