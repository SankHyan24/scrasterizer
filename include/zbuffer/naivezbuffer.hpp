#pragma once
#include <rasterizer/rasterizer.hpp>
#include <cassert>

class ZBuffer
{
};

class NaiveZBufferRaster : public Rasterizer
{
public:
    NaiveZBufferRaster(int width, int height, bool isGPU = true) : Rasterizer(width, height, isGPU) {}
    ~NaiveZBufferRaster() {}
    void render() override { assert(false && "NaiveZBufferRaster CPU Not Implemented!"); /*cpu not implemented*/ }
    void renderGPU() override
    {
        _autoRotateCamera();
        {
        }
        scene->drawGPU();
    }
    int renderInit() override
    {
        // disable depth test
        glDisable(GL_DEPTH_TEST);
        return 1;
    }

private:
};