#pragma once
#include <filesystem>

#include <core/camera.hpp>
#include <obj_loader/obj_loader.hpp>
#include <core/program.hpp>

class Scene
{
public:
    int width, height;
    // the only camera
    std::unique_ptr<Camera> camera;
    // objs
    std::vector<std::unique_ptr<OBJ>> objs;
    std::vector<bool> obj_activated;
    std::vector<std::string> obj_filenames;
    // programs for gpu
    std::vector<GLuint> textures;
    std::vector<std::string> texture_names;
    std::vector<std::unique_ptr<ShaderProgram>> programs;
    std::vector<std::vector<int>> program_obj_indices; // program index -> obj indices
    // compute shader programs
    std::vector<std::unique_ptr<ComputeShaderProgram>> compute_programs;
    std::vector<std::string> compute_program_names;

    // Functions
    //

    Scene(int width, int height, bool isGPU);
    ~Scene();

    void addOBJ(const std::string &filename, const std::string &shadername);
    void addOBJ(const std::string &filename);

    // get camera
    const Camera &getCamera() { return *camera; }
    Camera &getCameraV() { return *camera; }

    void transformObj(int index, const glm::mat4 &transform);  // implement transform for obj[index]
    void setObjModelMatrix(int index, const glm::mat4 &model); // set model matrix for obj[index]

    // for GPU buffer
    void createTexture(const std::string &texture_name);
    void bindTexture(const std::string &texture_name);
    void bindImageTexture(const std::string &texture_name, int index);
    void renderToTexture(const std::string &texture_name);
    void clearImageTexture(const std::string &texture_name);

    // for GPU render
    void loadShaders();
    void bindGPU();
    void drawGPU();
    void addComputeShader(const std::string &shadername);
    ComputeShaderProgram &getComputeProgram(const std::string &shadername); // get compute program reference by name
    // for debug
    void printDebugInfo();

private:
    // for gpu
    bool isGPU{false};
    GLuint framebuffer;

    void __createFrameBuffer();
    void __loadShadersLazy();
};