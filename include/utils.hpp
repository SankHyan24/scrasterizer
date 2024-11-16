#pragma once
#include <glm/glm.hpp>

namespace SCRA::Config
{
    extern const int HEIGHT;
    extern const int WIDTH;
}

namespace SCRA::Utils
{
    glm::mat4 TranslateMatrix(float x, float y, float z);
    glm::mat4 RotateMatrix(float x, float y, float z);
    glm::mat4 ScaleMatrix(float x, float y, float z);
    glm::mat4 ModelMatrix(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);
}