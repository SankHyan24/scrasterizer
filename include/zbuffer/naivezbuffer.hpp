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
        return 1;
    }

private:
};