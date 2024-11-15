#pragma once

#include <iostream>
#include <functional>
#include <GL/gl3w.h>
#include <imgui.h>
#include <glfw/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class Context
{
};
typedef Context *ctx;
using RenderFunc = std::function<int(void *, int, int)>;
typedef void (*CalcFunc)(ctx, void *rasterizer, int w, int h);

class Window
{
public:
    Window(int width, int height, const char *title) : width(width), height(height)
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width + 40, height + 40, title, NULL, NULL);
        if (!window)
        {
            std::cerr << "Failed to create window" << std::endl;
            glfwTerminate();
            exit(1);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        gl3wInit();

        SetupImGui();
    }

    ~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void update()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            renderFunc(&width, width, height);

            // Render the texture into the window
            showTextureMapImgui();
            ImGui::Render();
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }

    void bindTextureMap(char *textureMap)
    {
        this->textureMap = textureMap;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureMap);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void showTextureMapImgui()
    {
        ImGui::Begin("Image Display");
        ImGui::Image((ImTextureID)textureID, ImVec2(width, height));
        ImGui::End();
    }

    void showTextureMap()
    {
    }

    void setRenderFunc(RenderFunc renderFunc) { this->renderFunc = renderFunc; }

    void setCalcFunc(CalcFunc calcFunc) { this->calcFunc = calcFunc; }

    GLFWwindow *getWindow() { return window; }

private:
    void SetupImGui()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    GLFWwindow *window;
    int width, height;
    char *textureMap;
    // texture id
    GLuint textureID;
    RenderFunc renderFunc;
    CalcFunc calcFunc;
};