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
    for (int i = 0; i < scene->objs.size(); i++)
        if (scene->obj_activated[i])
            zbuffer->prepareVertex(*scene->objs[i], scene->getCameraV());
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

void NaiveZBufferRaster::__showZBufferDataStructInfo()
{
    _showimguiSubTitle("Naive Z-Buffer Info");
    ImGui::Text("Vertex Count: %d", zbuffer->vertices.size());
    ImGui::Text("Face Count: %d", zbuffer->faces.size());
    ImGui::Text("Triangle Count: %d", zbuffer->triangles.size());
    // use cull face
    ImGui::Checkbox("Use Cull Face", &zbuffer->use_cull_face);
    if (zbuffer->use_cull_face)
    {
        ImGui::Text("Culled Face Count: %d", zbuffer->culled_face);
    }
    else
    {
        ImGui::Text("Cull Face: Off");
    }
}
