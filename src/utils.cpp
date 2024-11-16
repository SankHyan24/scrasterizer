#include <utils.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SCRA::Utils
{
    glm::mat4 TranslateMatrix(float x, float y, float z)
    {
        return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    }

    glm::mat4 RotateMatrix(float x, float y, float z)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(z), glm::vec3(0.0f, 0.0f, 1.0f));
        return model;
    }

    glm::mat4 ScaleMatrix(float x, float y, float z)
    {
        return glm::scale(glm::mat4(1.0f), glm::vec3(x, y, z));
    }

    glm::mat4 ModelMatrix(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
}