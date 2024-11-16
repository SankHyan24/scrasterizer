#pragma once
#include <camera/camera.hpp>
#include <obj_loader/obj_loader.hpp>
#include <vector>
#include <memory>
#include <string>
class Scene
{
public:
    std::vector<std::unique_ptr<OBJ>> objs;
    std::vector<std::string> obj_filenames;
    std::unique_ptr<Camera> camera;

    Scene() : camera(std::make_unique<Camera>()) {}
    ~Scene() {}

    void addOBJ(std::string filename)
    {
        auto obj = std::make_unique<OBJLoader>(filename.c_str())->getOBJ();
        obj->printSummary();
        obj_filenames.push_back(obj->getFileName());
        objs.push_back(std::move(obj));
    }

    // get camera
    const Camera &getCamera() { return *camera; }
    Camera &getCameraV() { return *camera; }

    // transform obj
    void transformObj(int index, const glm::mat4 &transform)
    {
        if (index < 0 || index >= objs.size())
        {
            std::cerr << "Scene::transformObj index out of range" << std::endl;
            return;
        }
        objs[index]->implementTransform(transform);
    }
};