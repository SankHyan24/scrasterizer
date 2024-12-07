#include <utils.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SCRA::Config
{
    // not fixed
    const int HEIGHT = 800;
    const int WIDTH = 800;

    // fixed
    const int CS_LOCAL_SIZE_X = 16;
    const int CS_LOCAL_SIZE_Y = 16;
    const int CS_LOCAL_SIZE_Z = 1;

    //
    const float FloatMax = std::numeric_limits<float>::max();
    const float FloatMin = std::numeric_limits<float>::min();
    const float EPSILON = 1e-7;
}

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

    void saveAsPPM(char *RGB, int width, int height, std::string target_file_name)
    {
        FILE *fp = fopen(target_file_name.c_str(), "wb");
        // in opengl, the origin is at the bottom left corner. so we need to flip the image
        fprintf(fp, "P6\n%d %d\n255\n", width, height);
        for (int i = height - 1; i >= 0; i--)
            fwrite(RGB + i * width * 3, 1, width * 3, fp);
        fclose(fp);
    }

    void saveAsText(float *value, int width, int height, std::string target_file_name)
    {
        std::ofstream file(target_file_name);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                file << "[" << i << ", " << j << "] = " << value[i * width + j] << std::endl;
            }
            file << std::endl;
        }
    }

    float encodeCharsToFloat(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        unsigned char chars[4] = {r, g, b, a};
        float result;
        memcpy(&result, chars, sizeof(float));
        return result;
    }
}