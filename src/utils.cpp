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

    void saveAsPPM(float *RGB, int width, int height, std::string target_file_name)
    {
        char *RGB_char = new char[width * height * 3];
        for (int i = 0; i < width * height * 3; i++)
            RGB_char[i] = (char)(RGB[i] * 255);
        saveAsPPM(RGB_char, width, height, target_file_name);
        delete[] RGB_char;
    }

    void saveAsText(float *value, int channel, int width, int height, std::string target_file_name)
    {
        std::ofstream file(target_file_name);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                file << "[" << i << ", " << j << "] = ";
                for (int c = 0; c < channel; c++)
                    file << value[(i * width + j) * channel + c] << " ";
                file << std::endl;
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

    void decodeFloatToRGBA(float value, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a)
    {
        unsigned char chars[4];
        memcpy(chars, &value, sizeof(float));
        r = chars[0];
        g = chars[1];
        b = chars[2];
        a = chars[3];
    }
}