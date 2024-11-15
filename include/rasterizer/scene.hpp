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

    Scene()
    {
    }
    ~Scene() {}

    void addOBJ(std::string filename)
    {
        auto obj = std::make_unique<OBJLoader>(filename.c_str())->getOBJ();
        obj->printSummary();
        objs.push_back(std::move(obj));
        obj_filenames.push_back(filename);
    }
};