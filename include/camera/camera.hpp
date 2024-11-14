#pragma once
#include <glm/glm.hpp>
// TODO: Implement Camera class

// NCD: [0, 1]
class CameraParams
{
public:
    CameraParams(float fov, float aspect, float znear, float zfar) : fov(fov), aspect(aspect), znear(znear), zfar(zfar) {}
    CameraParams() : CameraParams(45.0f, 1.0f, 0.1f, 100.0f) {}
    ~CameraParams() {}
    float fov, aspect, znear, zfar;
};

enum class CameraProjectionType
{
    Perspective,
    Orthographic,
};

class Camera
{
public:
    Camera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up) : position(position), target(target), up(up) {}
    Camera() : Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)) {}
    ~Camera() {}
    void updateCamera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up);
    void updateCamera(const CameraParams &params);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;

private:
    glm::vec3 position, target, up;
    CameraParams params;
    CameraProjectionType projectionType{CameraProjectionType::Perspective};
};