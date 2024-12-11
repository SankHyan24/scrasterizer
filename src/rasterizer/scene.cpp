#include <rasterizer/scene.hpp>

//

Scene::Scene(int width, int height, bool isGPU) : camera(std::make_unique<Camera>()), isGPU(isGPU), width(width), height(height)
{
    if (isGPU)
    {
        __loadShadersLazy();
    }
}

Scene::~Scene() {}

void Scene::addOBJ(const std::string &filename)
{
    auto obj = std::make_unique<OBJLoader>(filename.c_str())->getOBJ();
    obj->printSummary();
    obj_filenames.push_back(obj->getFileName());
    objs.push_back(std::move(obj));
    obj_activated.push_back(true);
}

void Scene::addOBJ(const std::string &filename, const std::string &shadername)
{
    auto obj = std::make_unique<OBJLoader>(filename.c_str())->getOBJ();
    obj->printSummary();
    obj_filenames.push_back(obj->getFileName());
    objs.push_back(std::move(obj));
    obj_activated.push_back(true);
    // get program index
    int program_index = -1;
    for (int i = 0; i < programs.size(); i++)
        if (programs[i]->programName == shadername)
        {
            program_index = i;
            break;
        }
    if (program_index == -1)
    {
        std::cerr << "Scene::addOBJ shader name [" << shadername << "] not found" << std::endl;
        return;
    }
    program_obj_indices[program_index].push_back(objs.size() - 1); // now the programs have been loaded
}

void Scene::transformObj(int index, const glm::mat4 &transform)
{
    if (index < 0 || index >= objs.size())
    {
        std::cerr << "Scene::transformObj index out of range" << std::endl;
        return;
    }
    objs[index]->implementTransform(transform);
}

void Scene::setObjModelMatrix(int index, const glm::mat4 &model)
{
    if (index < 0 || index >= objs.size())
    {
        std::cerr << "Scene::setObjModelMatrix index out of range" << std::endl;
        return;
    }
    objs[index]->setModelMatrix(model);
}

void Scene::createTexture(const std::string &texture_name)
{
    texture_names.push_back(texture_name);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    textures.push_back(texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + textures.size() - 1, GL_TEXTURE_2D, texture, 0);
}

void Scene::bindTexture(const std::string &texture_name)
{
    int index = -1;
    for (int i = 0; i < texture_names.size(); i++)
        if (texture_names[i] == texture_name)
        {
            index = i;
            break;
        }
    if (index == -1)
    {
        std::cerr << SCRA::Utils::RED_LOG
                  << "Scene::bindTexture texture name [" << texture_name << "] not found"
                  << SCRA::Utils::COLOR_RESET
                  << std::endl;
        return;
    }
    glBindTexture(GL_TEXTURE_2D, textures[index]);
}

void Scene::bindImageTexture(const std::string &texture_name, int index)
{
    int texture_index = -1;
    for (int i = 0; i < texture_names.size(); i++)
        if (texture_names[i] == texture_name)
        {
            texture_index = i;
            break;
        }
    if (texture_index == -1)
    {
        std::cerr << SCRA::Utils::RED_LOG
                  << "Scene::bindImageTexture texture name [" << texture_name << "] not found"
                  << SCRA::Utils::COLOR_RESET
                  << std::endl;
        return;
    }
    glBindImageTexture(index, textures[texture_index], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
}

void Scene::renderToTexture(const std::string &texture_name)
{
    int index = -1;
    for (int i = 0; i < texture_names.size(); i++)
        if (texture_names[i] == texture_name)
        {
            index = i;
            break;
        }
    if (index == -1)
    {
        std::cerr << SCRA::Utils::RED_LOG
                  << "Scene::renderToTexture texture name [" << texture_name << "] not found"
                  << SCRA::Utils::COLOR_RESET
                  << std::endl;
        return;
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[index], 0);
}

void Scene::clearImageTexture(const std::string &texture_name)
{
    int index = -1;
    for (int i = 0; i < texture_names.size(); i++)
        if (texture_names[i] == texture_name)
        {
            index = i;
            break;
        }
    if (index == -1)
    {
        std::cerr << SCRA::Utils::RED_LOG
                  << "Scene::clearImageTexture texture name [" << texture_name << "] not found"
                  << SCRA::Utils::COLOR_RESET
                  << std::endl;
        return;
    }
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glClearTexImage(textures[index], 0, GL_RGB, GL_FLOAT, nullptr);
}

void Scene::loadShaders()
{
    __createFrameBuffer();
    for (auto &program : programs)
        program->init();
}

void Scene::bindGPU()
{ // called before render( in function "run")
    for (auto &obj : objs)
        obj->bindGPU();
}

void Scene::drawGPU()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // need to bindGPU first
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    for (auto &program : programs)
    {
        program->use();
        program->setUniformMat4("view", camera->getViewMatrix());
        program->setUniformMat4("projection", camera->getProjectionMatrix());

        for (auto &index : program_obj_indices[&program - &programs[0]])
        {
            program->setUniformMat4("model", objs[index]->getModelMatrix());
            objs[index]->drawGPU();
        }
    }
}

void Scene::addComputeShader(const std::string &shadername)
{
    auto compute_shader = std::make_unique<ComputeShaderProgram>("./shaders/" + shadername + "/compute.glsl", shadername);
    compute_shader->init();
    compute_programs.push_back(std::move(compute_shader));
    compute_program_names.push_back(shadername);
}

ComputeShaderProgram &Scene::getComputeProgram(const std::string &shadername)
{
    int index = -1;
    for (int i = 0; i < compute_program_names.size(); i++)
        if (compute_program_names[i] == shadername)
        {
            index = i;
            break;
        }
    if (index == -1)
    {
        std::cerr << SCRA::Utils::RED_LOG
                  << "Scene::getComputeProgram compute shader name [" << shadername << "] not found"
                  << SCRA::Utils::COLOR_RESET
                  << std::endl;
        exit(1);
    }
    return *compute_programs[index];
}

void Scene::printDebugInfo()
{
    std::cout << "Scene Debug Infomatoin:" << std::endl;
    std::cout << "\tCamera Info:" << std::endl;
    std::cout << "\t\tCamera Position: " << camera->getPosition().x << " " << camera->getPosition().y << " " << camera->getPosition().z << std::endl;
    std::cout << "\t\tCamera Target: " << camera->getTarget().x << " " << camera->getTarget().y << " " << camera->getTarget().z << std::endl;

    // about obj
    std::cout << "\tLoad " << objs.size() << " objs:" << std::endl;
    for (auto &obj : objs)
    {
        std::cout << "\t\tObj: " << obj->getFileName() << std::endl;
    }

    for (auto &program : programs)
    {
        std::cout << "\tUse program [" << program->programName << "] to render:" << std::endl;
        if (program_obj_indices[&program - &programs[0]].size() == 0)
        {
            std::cout << "\t\tNo obj use this program" << std::endl;
            continue;
        }
        for (auto &index : program_obj_indices[&program - &programs[0]])
        {
            std::cout << "\t\tObj: " << obj_filenames[index] << std::endl;
        }
    }
}

//
void Scene::__createFrameBuffer()
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void Scene::__loadShadersLazy()
{
    std::string shaders_folder_path = "./shaders";
    for (const auto &shader_folder : std::filesystem::directory_iterator(shaders_folder_path))
    {
        std::vector<std::string> shader_files;
        bool have_vertex = false, have_fragment = false;
        if (!shader_folder.is_directory())
            continue;
        for (const auto &file : std::filesystem::directory_iterator(shader_folder.path()))
        { // check if have vertex and fragment shader:vertex.glsl, fragment.glsl
            std::string file_name = file.path().filename().string();
            if (file_name == "vertex.glsl")
                have_vertex = true;
            else if (file_name == "fragment.glsl")
                have_fragment = true;
        }
        if (have_vertex && have_fragment)
        {
            std::string vertex_name = (shader_folder.path() / "vertex.glsl").string();
            std::string fragment_name = (shader_folder.path() / "fragment.glsl").string();
            std::string folder_name = shader_folder.path().filename().string();
            auto shader = new ShaderProgram(vertex_name, fragment_name, folder_name);
            programs.push_back(std::unique_ptr<ShaderProgram>(shader));
            program_obj_indices.push_back(std::vector<int>());
        }
    }
}