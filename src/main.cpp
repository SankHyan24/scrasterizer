#include <iostream>
#include <memory>
#include <window/glwindow.hpp>
#include <obj_loader/obj_loader.hpp>

void draw(const OBJ &obj)
{
    std::cout << "Drawing OBJ: " << obj.getFileName() << std::endl;
}
int main()
{
    int weight = 800;
    int height = 600;
    // char *map = new char[weight * height * 3];
    // auto window = Window(weight, height, "HelloWorld");
    // window.bindTextureMap(map);
    // RenderFunc screen = [](char *map, int w, int h)
    // {
    //     ImGui::Begin("CharMatrixVisualization");
    //     ImGui::End();
    // };
    // window.setRenderFunc(screen);
    // window.update();

    auto obj = std::make_unique<OBJLoader>("../assets/teapot.obj")->getOBJ();
}