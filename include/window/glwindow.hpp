#pragma once

#include <iostream>
#include <functional>
#include <GL/gl3w.h>
#include <imgui.h>
#include <glfw/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using RenderFunc = std::function<int(void *, int, int)>;

class Window
{
public:
    Window(int width, int height, const char *title);

    ~Window();

    void init();

    void update();

    void bindTextureMap(char *textureMap);

    void showTextureMapImgui();

    void setRenderFunc(RenderFunc renderFunc) { this->renderFunc = renderFunc; }

    GLFWwindow *getWindow() { return window; }

private:
    void SetupImGui();

    std::string title;
    GLFWwindow *window;
    int width, height;
    char *textureMap;
    // texture id
    GLuint textureID;
    RenderFunc renderFunc;
};