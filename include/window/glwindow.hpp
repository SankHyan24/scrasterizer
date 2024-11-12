#pragma once

#include <iostream>

#include <imgui.h>
#include <glfw/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void SetupImGui(GLFWwindow *window)
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

class Context
{
};
typedef Context *ctx;
typedef void (*RenderFunc)(char *, int w, int h);
typedef void (*CalcFunc)(ctx, char *, int w, int h);

class Window
{
public:
    Window(int width, int height, const char *title)
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!window)
        {
            std::cerr << "Failed to create window" << std::endl;
            glfwTerminate();
            exit(1);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        SetupImGui(window);
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
            renderFunc(textureMap, 800, 600);
            // Render the texture into the window
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
    }

    void setRenderFunc(RenderFunc renderFunc)
    {
        this->renderFunc = renderFunc;
    }

    void setCalcFunc(CalcFunc calcFunc)
    {
        this->calcFunc = calcFunc;
    }

    GLFWwindow *getWindow()
    {
        return window;
    }

private:
    GLFWwindow *window;
    char *textureMap;
    RenderFunc renderFunc;
    CalcFunc calcFunc;
};