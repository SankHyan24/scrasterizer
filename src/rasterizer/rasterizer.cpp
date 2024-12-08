#include <rasterizer/rasterizer.hpp>

void Rasterizer::init(int width, int height)
{
    this->width = width;
    this->height = height;
    // create scene, canvas, window
    scene = std::make_unique<Scene>(width, height, isGPU); // to obtain OBJs and camera
    canvas = std::make_unique<Canvas>(width, height);
    window = std::make_unique<Window>(width, height, class_name.c_str());
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
}

void Rasterizer::run()
{
    if (isGPU)
    {
        scene->printDebugInfo();
        scene->bindGPU();
    }
    window->run();
};

void Rasterizer::implementTransform(std::string file_name, const glm::mat4 &transform)
{
    // Translate, Rotate, Scale
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

void Rasterizer::_autoRotateCamera(float v)
{
    // rotate camera around target
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
    auto textureMap = canvas->getTextureMap();
    int width = canvas->getWidth();
    int height = canvas->getHeight();

    // from screen space[-1,1] to pixel space
    auto p0x = (p0.x + 1.0f) * 0.5f * width;
    auto p0y = (p0.y + 1.0f) * 0.5f * height;
    auto p1x = (p1.x + 1.0f) * 0.5f * width;
    auto p1y = (p1.y + 1.0f) * 0.5f * height;

    // Bresenham's line algorithm
    int x0 = p0x, y0 = p0y;
    int x1 = p1x, y1 = p1y;
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