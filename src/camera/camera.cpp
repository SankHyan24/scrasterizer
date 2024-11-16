#include <camera/camera.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Camera::updateCamera(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up)
{
    this->position = position;
    this->target = target;
    this->up = up;
    _reCalcMatrix();
}

void Camera::updateCamera(const CameraParams &params)
{
    this->params = params;
    _reCalcMatrix();
}

void Camera::movePosition(const glm::vec3 &position)
{
    this->position += position;
    _reCalcMatrix();
}

void Camera::moveTarget(const glm::vec3 &target)
{
    this->target += target;
    _reCalcMatrix();
}

void Camera::_reCalcMatrix()
{
    if (projectionType == CameraProjectionType::Perspective)
        projectionMatrix = glm::perspective(glm::radians(params.fov), params.aspect, params.znear, params.zfar);
    else
        projectionMatrix = glm::ortho(-params.width, params.width, -params.width * params.aspect, params.width * params.aspect, params.znear, params.zfar);
    viewMatrix = glm::lookAt(position, target, up);
    viewProjectionMatrix = projectionMatrix * viewMatrix;
}

const glm::mat4 &Camera::getViewMatrix() const
{
    return viewMatrix;
}

const glm::mat4 &Camera::getProjectionMatrix() const
{
    return projectionMatrix;
}

const glm::mat4 &Camera::getViewProjectionMatrix() const
{
    return viewProjectionMatrix;
}

void Camera::calcDebug()
{
    viewMatrix = glm::lookAt(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    projectionMatrix = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 1.0f, 100.0f);
    // projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10000.0f);
    viewProjectionMatrix = projectionMatrix * viewMatrix;
}
