#include <glm/gtc/constants.hpp>
#include <rasterizer/rasterizer.hpp>
#include <utils.hpp>
void Rasterizer::init(int width, int height)
{
#ifdef _MSC_VER
    std::cout << "Use " << SCRA::Utils::BLUE_LOG << "Microsoft Visual C++ Compiler" << SCRA::Utils::COLOR_RESET << std::endl;
#else
    std::cout << "Use " << SCRA::Utils::BLUE_LOG << "GNU Compiler" << SCRA::Utils::COLOR_RESET << std::endl;
#endif
    this->width = width;
    this->height = height;
    this->__printHello();
    // create scene, canvas, window
    scene = std::make_unique<Scene>(width, height, isGPU); // to obtain OBJs and camera
    canvas = std::make_unique<Canvas>(width, height);
    window = std::make_unique<Window>(width, height, isGPU);
    window->init(); // opengl, imgui setup
    if (isGPU)
        scene->loadShaders(); // compile shaders
    else
        window->bindTextureMap(canvas->getTextureMap()); // use cpu array to render

    // set render function
    RenderFunc screen = [this]() -> bool
    {
        if (this->isGPU)
        {
            this->renderGPU();
        }
        else
        {
            canvas->clearTextureMap();
            this->render();
        }
        return this->isGPU;
    };
    window->setRenderCallback(screen);

    // set render init function
    RenderFunc init = [this]() -> bool
    {
        return this->renderInit();
    };
    window->setRenderInitFunc(init);

    // set render imgui function
    RenderFunc imgui = [this]() -> bool
    {
        return this->renderImGui();
    };
    window->setRenderImGui(imgui);
}

void Rasterizer::run()
{ // this function only run once
    for (auto &obj : scene->objs)
        modelParams.push_back(SCRA::Utils::ModelParams());
    if (isGPU)
    {
        scene->printDebugInfo();
        scene->bindGPU();
    }
    window->run(); // loop is inside this function
};

void Rasterizer::implementTransform(std::string file_name, const glm::mat4 &transform)
{
    // Translate, Rotate, Scale, this change the obj file permanently
    int index = -1;
    for (int i = 0; i < scene->obj_filenames.size(); i++)
        if (scene->obj_filenames[i] == file_name)
        {
            index = i;
            break;
        }
    if (index == -1)
    {
        std::cerr << "Rasterizer::implementTransform file_name not found" << std::endl;
        return;
    }
    scene->transformObj(index, transform);
}

void Rasterizer::loadOBJ(const std::string &filename)
{
    assert(isGPU == false && "You use GPU mode, obj need corresponding shader. Please use loadOBJ(const std::string &filename, const std::string &shadername) instead!");
    scene->addOBJ(filename);
}

void Rasterizer::_autoRotateCamera()
{
    // rotate camera around target
    float v = camAutoRotateSpeed;
    float Dx = scene->getCameraV().getPosition().x - scene->getCameraV().getTarget().x;
    float Dz = scene->getCameraV().getPosition().z - scene->getCameraV().getTarget().z;
    float radius = sqrt(Dx * Dx + Dz * Dz);
    float angle = atan2(Dz, Dx);
    angle += glm::radians(v);
    float dx = radius * cos(angle);
    float dz = radius * sin(angle);
    auto &camera = scene->getCameraV();
    camera.movePosition(glm::vec3(dx, camera.getPosition().y, dz) - camera.getPosition());
}

void Rasterizer::_setCameraTheta()
{
    // theta is from 0 to 1, now to 0 to 2pi
    float theta = camHoriTheta * glm::pi<float>();
    glm::vec3 camVector(scene->getCameraV().getPosition() - scene->getCameraV().getTarget());
    float radius = glm::length(camVector);
    float x = camVector.x; // red axis
    float y = camVector.y; // green axis
    float z = camVector.z; // blue axis
    float new_x, new_y, new_z;

    if (z == 0.0f)
    {
        new_x = glm::cos(theta) * radius;
        new_y = glm::sin(theta) * radius;
        new_z = 0.0f;
    }
    else
    {
        float dxdz = x / z;
        float sqrt_ = sqrt(1 + dxdz * dxdz);
        new_y = glm::cos(theta) * radius;
        float new_xz = glm::sin(theta) * radius;
        new_z = new_xz / sqrt_;
        new_x = dxdz * new_z;
    }
    scene->getCameraV().updateCamera(glm::vec3(new_x, new_y, new_z) + scene->getCameraV().getTarget(), scene->getCameraV().getTarget(), scene->getCameraV().getUp());
}

void Rasterizer::_setCameraLengthToTarget()
{
    auto &camera = scene->getCameraV();
    auto position = camera.getPosition();
    auto target = camera.getTarget();
    auto camVector = position - target;
    float radius = glm::length(camVector);
    auto newCamVector = glm::normalize(camVector) * camLengthToTarget;
    camera.updateCamera(target + newCamVector, target, camera.getUp());
}

void Rasterizer::_setModelMatrix(int obj_index)
{
    glm::mat4 modelMatrix(1.0f);
    SCRA::Utils::ModelParams &modelParam = modelParams[obj_index];
    modelMatrix = SCRA::Utils::TranslateMatrix(modelParam.x, modelParam.y, modelParam.z) * modelMatrix;
    modelMatrix = SCRA::Utils::RotateMatrix(modelParam.rx, modelParam.ry, modelParam.rz) * modelMatrix;
    modelMatrix = SCRA::Utils::ScaleMatrix(modelParam.sx, modelParam.sy, modelParam.sz) * modelMatrix;
    scene->setObjModelMatrix(obj_index, modelMatrix);
}

void Rasterizer::_drawCoordinateAxis()
{
    auto &camera = scene->getCameraV();
    auto position = camera.getPosition();
    auto target = camera.getTarget();
    auto viewProjectionMatrix = camera.getViewProjectionMatrix();
    // x axis
    auto p0 = glm::vec3(0.0f, 0.0f, 0.0f);
    auto p1 = glm::vec3(1.0f, 0.0f, 0.0f);
    auto p0x = glm::vec4(viewProjectionMatrix * glm::vec4(p0.x, p0.y, p0.z, 1.0f));
    p0x.x = p0x.x / p0x.w;
    p0x.y = p0x.y / p0x.w;
    auto p1x = glm::vec4(viewProjectionMatrix * glm::vec4(p1.x, p1.y, p1.z, 1.0f));
    p1x.x = p1x.x / p1x.w;
    p1x.y = p1x.y / p1x.w;
    _drawLine(glm::vec2(p0x.x, p0x.y), glm::vec2(p1x.x, p1x.y), 255, 0, 0);
    // y axis
    auto p2 = glm::vec3(0.0f, 0.0f, 0.0f);
    auto p3 = glm::vec3(0.0f, 1.0f, 0.0f);
    auto p2y = glm::vec4(viewProjectionMatrix * glm::vec4(p2.x, p2.y, p2.z, 1.0f));
    p2y.x = p2y.x / p2y.w;
    p2y.y = p2y.y / p2y.w;
    auto p3y = glm::vec4(viewProjectionMatrix * glm::vec4(p3.x, p3.y, p3.z, 1.0f));
    p3y.x = p3y.x / p3y.w;
    p3y.y = p3y.y / p3y.w;
    _drawLine(glm::vec2(p2y.x, p2y.y), glm::vec2(p3y.x, p3y.y), 0, 255, 0);
    // z axis
    auto p4 = glm::vec3(0.0f, 0.0f, 0.0f);
    auto p5 = glm::vec3(0.0f, 0.0f, 1.0f);
    auto p4z = glm::vec4(viewProjectionMatrix * glm::vec4(p4.x, p4.y, p4.z, 1.0f));
    p4z.x = p4z.x / p4z.w;
    p4z.y = p4z.y / p4z.w;
    auto p5z = glm::vec4(viewProjectionMatrix * glm::vec4(p5.x, p5.y, p5.z, 1.0f));
    p5z.x = p5z.x / p5z.w;
    p5z.y = p5z.y / p5z.w;
    _drawLine(glm::vec2(p4z.x, p4z.y), glm::vec2(p5z.x, p5z.y), 0, 0, 255);
}

void Rasterizer::_drawLine(const glm::vec2 &p0, const glm::vec2 &p1, char r, char g, char b)
{
    // from screen space[-1,1] to pixel space
    auto p0x = (p0.x + 1.0f) * 0.5f * width;
    auto p0y = (p0.y + 1.0f) * 0.5f * height;
    auto p1x = (p1.x + 1.0f) * 0.5f * width;
    auto p1y = (p1.y + 1.0f) * 0.5f * height;
    _drawLineScreenSpace(glm::vec2(p0x, p0y), glm::vec2(p1x, p1y), r, g, b);
}
void Rasterizer::_drawLineScreenSpace(const glm::vec2 &p0, const glm::vec2 &p1, char r, char g, char b)
{
    auto textureMap = canvas->getTextureMap();
    int width = canvas->getWidth();
    int height = canvas->getHeight();

    // Bresenham's line algorithm
    int x0 = p0.x, y0 = p0.y;
    int x1 = p1.x, y1 = p1.y;
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true)
    {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height)
        {
            textureMap[(y0 * width + x0) * 3] = r;
            textureMap[(y0 * width + x0) * 3 + 1] = g;
            textureMap[(y0 * width + x0) * 3 + 2] = b;
        }
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void Rasterizer::_drawTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
    _drawLine(glm::vec2(v0.x, v0.y), glm::vec2(v1.x, v1.y));
    _drawLine(glm::vec2(v1.x, v1.y), glm::vec2(v2.x, v2.y));
    _drawLine(glm::vec2(v2.x, v2.y), glm::vec2(v0.x, v0.y));
}

void Rasterizer::_showCameraInfoInImgui()
{

    _showimguiSubTitle("Camera Info");
    auto &camera = scene->getCamera();
    auto &cameraV = scene->getCameraV();
    {
        ImGui::Text("Auto Rotate Speed:");
        ImGui::SameLine();
        ImGui::SliderFloat("##slider0", &camAutoRotateSpeed, 0.0f, 3.0f);
    }
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
    ImGui::Text("Target: (%.2f, %.2f, %.2f)", camera.getTarget().x, camera.getTarget().y, camera.getTarget().z);
    ImGui::Text("Up: (%.2f, %.2f, %.2f)", camera.getUp().x, camera.getUp().y, camera.getUp().z);
    ImGui::Separator();
    {
        // set camera position
        ImGui::Text("Camera Height:");
        ImGui::SameLine();
        ImGui::SliderFloat("##slider1", &camHoriTheta, 0.001f, 0.999f);
        if (camHoriTheta != camHoriThetaOld)
        {
            camHoriThetaOld = camHoriTheta;
            _setCameraTheta();
        }
    }
    {
        // set camera length to target
        ImGui::Text("Camera Length:");
        ImGui::SameLine();
        ImGui::SliderFloat("##slider2", &camLengthToTarget, 0.1f, 20.0f);
        if (camLengthToTarget != camLengthToTargetOld)
        {
            camLengthToTargetOld = camLengthToTarget;
            _setCameraLengthToTarget();
        }
    }
}

void Rasterizer::_showObjInfosInImgui()
{
    _showimguiSubTitle("OBJ Info");
    ImGui::Text("OBJ Count: %d", scene->objs.size());
    for (int i = 0; i < scene->objs.size(); i++)
    {
        ImGui::Separator();
        auto &obj = scene->objs[i];
        bool obj_activated = scene->obj_activated[i];
        std::string obj_name = obj->getFileName();
        if (ImGui::Checkbox(obj_name.c_str(), &obj_activated))
        {
            scene->obj_activated[i] = obj_activated;
        }
        if (!obj_activated)
            continue;
        ImGui::PushID(i);

        ImGui::Text("OBJ [%d]: %s", i, obj->getFileName().c_str());
        ImGui::Text("Vertex Count: %d", obj->getVertices().size());
        ImGui::Text("Face Count: %d", obj->getFaces().size());
        // transform
        {
            static SCRA::Utils::ModelParams modelParam;
            std::string label = "##xx" + std::to_string(i);
            ImGui::Text("Transform");
            ImGui::SameLine();
            if (ImGui::Button("Reset"))
            {
                modelParam = SCRA::Utils::ModelParams();
                modelParams[i] = modelParam;
                _setModelMatrix(i);
            }
            ImGui::InputFloat3((label + "Translate").c_str(), &modelParam.x);
            ImGui::SameLine();
            if (ImGui::Button("Translate"))
            {
                modelParams[i].x = modelParam.x;
                modelParams[i].y = modelParam.y;
                modelParams[i].z = modelParam.z;
                _setModelMatrix(i);
            }
            ImGui::InputFloat3((label + "Rotate").c_str(), &modelParam.rx);
            ImGui::SameLine();
            if (ImGui::Button("Rotate"))
            {
                modelParams[i].rx = modelParam.rx;
                modelParams[i].ry = modelParam.ry;
                modelParams[i].rz = modelParam.rz;
                _setModelMatrix(i);
            }
            ImGui::InputFloat3((label + "Scale").c_str(), &modelParam.sx);
            ImGui::SameLine();
            if (ImGui::Button("Scale"))
            {
                modelParams[i].sx = modelParam.sx;
                modelParams[i].sy = modelParam.sy;
                modelParams[i].sz = modelParam.sz;
                _setModelMatrix(i);
            }
        }
        ImGui::PopID();
    }
}

void Rasterizer::_showimguiSubTitle(const std::string &title)
{
    ImGui::Separator();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::CalcTextSize(title.c_str());

    ImGui::GetWindowDrawList()->AddRectFilled(
        cursorPos,
        ImVec2(cursorPos.x + size.x, cursorPos.y + size.y),
        IM_COL32(50, 50, 150, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 105, 255));
    ImGui::Text(title.c_str());
    ImGui::PopStyleColor();
    ImGui::Separator();
}

void Rasterizer::__printHello()
{
    std::cerr << "Width: " << width << " Height: " << height << std::endl;
    std::cerr << "Using " << SCRA::Utils::GREEN_LOG << (isGPU ? "GPU" : "CPU") << SCRA::Utils::COLOR_RESET << " mode" << std::endl;
}
