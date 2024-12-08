#include <zbuffer/scanline.hpp>
ScanlineRaster::ScanlineRaster(int width, int height, bool isGPU) : Rasterizer(width, height, isGPU)
{
    scanline = std::make_unique<Scanline>(width, height);
    zBufferPrecompute = scanline->zBufferData;
}

ScanlineRaster::~ScanlineRaster()
{
    delete[] zBufferPrecompute;
}

void ScanlineRaster::render()
{
    _autoRotateCamera();
    auto textureMap = canvas->getTextureMap();
    __ScanLinePreCompute();
    for (int i = 0; i < width * height; i++)
    { // if textureMap is not zero
        unsigned char r, g, b, a;
        SCRA::Utils::decodeFloatToRGBA(zBufferPrecompute[i], r, g, b, a);
        textureMap[i * 3] = r;
        textureMap[i * 3 + 1] = g;
        textureMap[i * 3 + 2] = b;
    }
    _drawCoordinateAxis();
}

void ScanlineRaster::renderGPU()
{
    _autoRotateCamera();
    {
        auto program = scene->getComputeProgram("scanline");
        program.use();
        __ScanLinePreCompute();
        glDispatchCompute(this->width / SCRA::Config::CS_LOCAL_SIZE_X, this->height / SCRA::Config::CS_LOCAL_SIZE_Y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    scene->drawGPU();
    scene->clearImageTexture("imgOutput");
}

int ScanlineRaster::renderInit()
{
    if (isGPU)
    {
        // disable depth test
        glDisable(GL_DEPTH_TEST);

        // use compute program to render
        this->addComputeShader("scanline");
        scene->createTexture("imgOutput");

        // bind texture to shader
        auto program = scene->getComputeProgram("scanline");
        program.use();
        scene->bindImageTexture("imgOutput", 0);
        __ScanLinePreInit();
    }
    return 1;
}

void ScanlineRaster::__ScanLinePreInit()
{
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, width * height * sizeof(float), zBufferPrecompute, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ScanlineRaster::__ScanLinePreCompute()
{
    scanline->init();
    for (auto &obj : scene->objs)
        scanline->buildTable(*obj, scene->getCameraV());
    scanline->scanScreen();

    if (isGPU)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, width * height * sizeof(float), zBufferPrecompute);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void ScanlineRaster::__ScanLinePreDebug()
{
    debug_count++;
    if (debug_count != 0)
        return;
    auto color = new char[width * height * 3];
    for (int i = 0; i < width * height; i++)
    {
        color[i * 3] = zBufferPrecompute[i] * 10;
        color[i * 3 + 1] = zBufferPrecompute[i] * 10;
        color[i * 3 + 2] = zBufferPrecompute[i] * 10;
    }
    SCRA::Utils::saveAsText(zBufferPrecompute, width, height, "zbuffer.txt");
    SCRA::Utils::saveAsPPM(color, width, height, "zbuffer.ppm");
    delete[] color;
}
