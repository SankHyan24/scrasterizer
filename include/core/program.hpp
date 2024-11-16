#pragma once
#include <GL/gl3w.h>
#include <string>
#include <fstream>
#include <sstream>

class ShaderProgram
{
public:
    GLuint programID;
    std::string programName;
    ShaderProgram(const std::string &vertexPath, const std::string &fragmentPath, const std::string &programName) : programName(programName)
    {
        vertexCode = __loadCode(vertexPath);
        fragmentCode = __loadCode(fragmentPath);
    };
    ~ShaderProgram() {};
    void init()
    {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char *vCode = vertexCode.c_str();
        glShaderSource(vertexShader, 1, &vCode, NULL);
        glCompileShader(vertexShader);
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ShaderProgram::init vertex shader compilation failed\n"
                      << infoLog << std::endl;
        }
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *fCode = fragmentCode.c_str();
        glShaderSource(fragmentShader, 1, &fCode, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ShaderProgram::init fragment shader compilation failed\n"
                      << infoLog << std::endl;
        }
        programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID);
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programID, 512, NULL, infoLog);
            std::cerr << "ShaderProgram::init program linking failed\n"
                      << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    } // compile and link
    void use() { glUseProgram(programID); }

private:
    std::string vertexCode;
    std::string fragmentCode;
    std::string __loadCode(const std::string &path)
    {
        std::string code;
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            file.open(path);
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            code = stream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cerr << "ShaderProgram::__loadCode failed to read file " << path << std::endl;
        }
        return code;
    }
};