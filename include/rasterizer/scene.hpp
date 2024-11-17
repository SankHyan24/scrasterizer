#pragma once
#include <filesystem>

#include <core/camera.hpp>
#include <obj_loader/obj_loader.hpp>
#include <core/program.hpp>

class Scene
{
public:
    // the only camera
    std::unique_ptr<Camera> camera;
    // objs
    std::vector<std::unique_ptr<OBJ>> objs;
    std::vector<std::string> obj_filenames;
    // programs
    std::vector<std::unique_ptr<ShaderProgram>> programs;
    std::vector<std::vector<int>> program_obj_indices; // program index -> obj indices

    Scene() : camera(std::make_unique<Camera>()) { loadShadersLazy(); }
    ~Scene() {}

    void addOBJ(const std::string &filename, const std::string &shadername = "basic")
    {
        auto obj = std::make_unique<OBJLoader>(filename.c_str())->getOBJ();
        obj->printSummary();
        obj_filenames.push_back(obj->getFileName());
        objs.push_back(std::move(obj));
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

    // get camera
    const Camera &getCamera() { return *camera; }
    Camera &getCameraV() { return *camera; }

    void transformObj(int index, const glm::mat4 &transform)
    {
        if (index < 0 || index >= objs.size())
        {
            std::cerr << "Scene::transformObj index out of range" << std::endl;
            return;
        }
        objs[index]->implementTransform(transform);
    }
    void loadShaders()
    {
        for (auto &program : programs)
            program->init();
    }
    void bindGPU()
    { // called before render( in function "run")
        for (auto &obj : objs)
            obj->bindGPU();
        // bind camera
    }

    void drawGPU()
    { // need to bindGPU first
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

private:
    void loadShadersLazy()
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
};