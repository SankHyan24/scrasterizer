#include <zbuffer/naivezbuffer.hpp>

NaiveZBufferRaster::NaiveZBufferRaster(int width, int height, bool isGPU) : Rasterizer(width, height, isGPU)
{
    zbuffer = std::make_unique<NaiveZBuffer>(width, height);
    zBufferPrecompute = zbuffer->zBufferData;
    colorPrecompute = zbuffer->colorBufferData;
}

void NaiveZBufferRaster::render()
{
    _autoRotateCamera();
    zbuffer->init();
    for (auto &obj : scene->objs)
        zbuffer->prepareVertex(*obj, scene->getCameraV());
    zbuffer->drawFragment();
    __putColorBuffer2TextureMap();
    _drawCoordinateAxis();
}

void NaiveZBufferRaster::__putColorBuffer2TextureMap()
{
    auto textureMap = canvas->getTextureMap();
    for (int i = 0; i < width * height * 3; i++)
    {
        float climped = colorPrecompute[i] > 1.0f ? 1.0f : colorPrecompute[i];
        climped = climped < 0.0f ? 0.0f : climped;
        textureMap[i] = (unsigned char)(int)(climped * 255);
    }
}