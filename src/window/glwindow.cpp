#include <window/glwindow.hpp>

Window::Window(int width, int height, bool isGPU, const char *title) : width(width), height(height), isGPU(isGPU), title(title)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::init()
{

    window = glfwCreateWindow(width + (isGPU ? 0 : 520), height + (isGPU ? 0 : 40), title.c_str(), NULL, NULL);
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

    // z-buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void Window::run()
{
    if (renderInitFunc)
        renderInitFunc();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    if (!renderCallback())       // Our Render function
        __showTextureMapImgui(); // Render the texture into the window

    __showImGuiSubWindow();
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
    auto textureMap_ = new char[width * height * 3];
    for (int i = 0; i < height; i++)
    {
        int y = height - i - 1;
        memcpy(textureMap_ + y * width * 3, textureMap + i * width * 3, width * 3);
    }
    memcpy(textureMap, textureMap_, width * height * 3);
    delete[] textureMap_;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureMap);
    glBindTexture(GL_TEXTURE_2D, 0);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(width + 20, height + 40), ImGuiCond_FirstUseEver);
    ImGui::Begin("Display");
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

void Window::__showImGuiSubWindow()
{
    ImGui::SetNextWindowPos(ImVec2(width + 20, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, height + 40), ImGuiCond_FirstUseEver);
    ImGui::Begin("Control Panel");
    if (show_FPS)
        __showFPS();
    if (renderImGui)
        renderImGui();
    ImGui::End();
}

void Window::__showFPS()
{
    ImGui::Text("FPS: %.1f", fpsNow); // 显示当前帧率
}
