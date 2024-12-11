#pragma once
#include <glm/glm.hpp>
#include <string>
#include <fstream>
namespace SCRA::Config
{
    extern const int HEIGHT;
    extern const int WIDTH;

    extern const int CS_LOCAL_SIZE_X;
    extern const int CS_LOCAL_SIZE_Y;
    extern const int CS_LOCAL_SIZE_Z;

    extern const float FloatMax;
    extern const float FloatMin;
    extern const float EPSILON;
}

namespace SCRA::Utils
{
    const std::string RED_LOG = "\033[1;31m";
    const std::string GREEN_LOG = "\033[1;32m";
    const std::string COLOR_RESET = "\033[0m";

    glm::mat4 TranslateMatrix(float x, float y, float z);
    glm::mat4 RotateMatrix(float x, float y, float z);
    glm::mat4 ScaleMatrix(float x, float y, float z);
    glm::mat4 ModelMatrix(const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale);

    struct ModelParams
    {
        float x, y, z;
        float rx, ry, rz;
        float sx, sy, sz;
        ModelParams() : x(0.0f), y(0.0f), z(0.0f), rx(0.0f), ry(0.0f), rz(0.0f), sx(1.0f), sy(1.0f), sz(1.0f) {}
    };

    void saveAsPPM(char *RGB, int width, int height, std::string target_file_name);
    void saveAsPPM(float *RGB, int width, int height, std::string target_file_name);
    void saveAsText(float *value, int channel, int width, int height, std::string target_file_name);

    float encodeCharsToFloat(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    void decodeFloatToRGBA(float value, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a);
}
