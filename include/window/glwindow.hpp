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

    void run();

    void bindTextureMap(char *textureMap);

    void setRenderFunc(RenderFunc renderFunc) { this->renderFunc = renderFunc; }

    GLFWwindow *getWindow() { return window; }

private:
    void __update();
    void __showTextureMapImgui();
    void __setupImGui();

    // Window attributes
    std::string title;
    int width, height;
    GLFWwindow *window;

    // Frame Counter
    double lastTime{0.0}, deltaTime{0.0};
    int frameCount{0};
    double fpsNow{0.0};

    // Use to bind texture map
    char *textureMap;
    GLuint textureID;

    // Render interface
    RenderFunc renderFunc;
};