#include <iostream>
#include <window/glwindow.hpp>
int main()
{
    int weight = 800;
    int height = 600;
    char *map = new char[weight * height * 3];
    auto window = Window(weight, height, "HelloWorld");
    window.bindTextureMap(map);

    RenderFunc screen = [](char *map, int w, int h)
    {
        ImGui::Begin("CharMatrixVisualization");
        ImGui::End();
    };
    window.setRenderFunc(screen);
    window.update();
}