#include <camera/camera.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Camera::updateCamera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
    this->position = position;
    this->target = target;
    this->up = up;
}

void Camera::updateCamera(const CameraParams &params)
{
    this->params = params;
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    if (projectionType == CameraProjectionType::Perspective)
        return glm::perspective(glm::radians(params.fov), params.aspect, params.znear, params.zfar);
    else
        return glm::ortho(-params.aspect, params.aspect, -1.0f, 1.0f, params.znear, params.zfar);
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
    return getProjectionMatrix() * getViewMatrix();
}
