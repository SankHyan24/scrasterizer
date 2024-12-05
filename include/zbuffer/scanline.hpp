#pragma once
#include <rasterizer/rasterizer.hpp>
#include <cassert>
class ScanlineRaster : public Rasterizer
{
public:
    ScanlineRaster(int width, int height, bool isGPU = true) : Rasterizer(width, height, isGPU) {}

    ~ScanlineRaster() {}
    void render() override { assert(false); /*not implemented*/ }

    void renderGPU() override
    {
        _autoRotateCamera();
        {
            auto program = scene->getComputeProgram("debug");
            program.use();
            glDispatchCompute(this->width / SCRA::Config::CS_LOCAL_SIZE_X, this->height / SCRA::Config::CS_LOCAL_SIZE_Y, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        scene->drawGPU();
    }
    int renderInit() override
    {
        // disable depth test
        glDisable(GL_DEPTH_TEST);

        // use compute program to render
        scene->createTexture("imgOutput");

        // bind texture to shader
        {
            auto program = scene->getComputeProgram("debug");
            program.use();
            scene->bindImageTexture("imgOutput", 0);
        }

        return 1;
    }

private:
    void ScanLinePrecompute()
    {
        // precompute the scanline
        std::vector<float> intersections;
    }
};