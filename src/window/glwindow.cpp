#include <window/glwindow.hpp>

Window::Window(int width, int height, const char *title) : width(width), height(height), title(title)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::init()
{
    window = glfwCreateWindow(width + 40, height + 40, title.c_str(), NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gl3wInit();

    __setupImGui();

    // sc
}

void Window::run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::NewFrame();

        __update();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void Window::__update()
{
    frameCount++;
    double currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    if (deltaTime >= 1.0)
    {
        fpsNow = frameCount / deltaTime;
        frameCount = 0;
        lastTime = currentTime;
    }
    if (!renderFunc())           // Our Render function
        __showTextureMapImgui(); // Render the texture into the window
}

void Window::bindTextureMap(char *textureMap)
{
    this->textureMap = textureMap;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureMap);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Window::__showTextureMapImgui()
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureMap);
    glBindTexture(GL_TEXTURE_2D, 0);
    ImGui::Begin("Display");
    ImGui::Text("FPS: %.1f", fpsNow); // 显示当前帧率
    ImGui::Image((ImTextureID)textureID, ImVec2(width, height));
    ImGui::End();
}

void Window::__setupImGui()
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

//

void Window::__showWindow()
{
    // 负责绘制窗口内容，而不是imgui
}