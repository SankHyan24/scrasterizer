#pragma once
#include <glm/glm.hpp>
// NCD: [-1, 1]

namespace
{
    const float widthDefault = 4.0;
}
class CameraParams
{
public:
    CameraParams(float fov, float aspect, float znear, float zfar) : fov(fov), aspect(aspect), znear(znear), zfar(zfar) {}
    CameraParams() : CameraParams(45.0f, 1.0f, 0.001f, 1000.0f) {}
    ~CameraParams() {}
    float fov, aspect, znear, zfar;
    float width{widthDefault};
};

enum class CameraProjectionType
{
    Perspective,
    Orthographic,
};

class Camera
{
public:
    Camera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up) : position(position), target(target), up(up) { _reCalcMatrix(); }
    Camera() : Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) { _reCalcMatrix(); }
    ~Camera() {}
    void updateCamera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up);
    void updateCamera(const CameraParams &params);

    void movePosition(const glm::vec3 &position);
    void moveTarget(const glm::vec3 &target);

    void usePerspective() { projectionType = CameraProjectionType::Perspective; }
    void useOrthographic(int width = widthDefault)
    {
        params.width = width;
        projectionType = CameraProjectionType::Orthographic;
    }

    const glm::mat4 &getViewMatrix() const;
    const glm::mat4 &getProjectionMatrix() const;
    const glm::mat4 &getViewProjectionMatrix() const;

    const glm::vec3 &getPosition() const { return position; }
    const glm::vec3 &getTarget() const { return target; }
    const glm::vec3 &getUp() const { return up; }

    void calcDebug();

private:
    void _reCalcMatrix();
    glm::vec3 position, target, up;
    CameraParams params;
    CameraProjectionType projectionType{CameraProjectionType::Perspective};
    glm::mat4 viewMatrix, projectionMatrix;
    glm::mat4 viewProjectionMatrix;
};