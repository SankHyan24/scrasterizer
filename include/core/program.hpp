#pragma once
#include <GL/gl3w.h>
#include <string>
#include <fstream>
#include <sstream>

#include <utils.hpp>

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
            std::cerr << SCRA::Utils::RED_LOG << "ShaderProgram::init vertex shader compilation failed\n"
                      << infoLog << SCRA::Utils::COLOR_RESET << std::endl;
        }
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *fCode = fragmentCode.c_str();
        glShaderSource(fragmentShader, 1, &fCode, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << SCRA::Utils::RED_LOG << "ShaderProgram::init fragment shader compilation failed\n"
                      << infoLog << SCRA::Utils::COLOR_RESET << std::endl;
        }
        programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID);
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programID, 512, NULL, infoLog);
            std::cerr << SCRA::Utils::RED_LOG << "ShaderProgram::init program linking failed\n"
                      << infoLog << SCRA::Utils::COLOR_RESET << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    } // compile and link
    void use() { glUseProgram(programID); }
    GLuint getProgramID() { return programID; }
    GLint getUniformLocation(const std::string &name) { return glGetUniformLocation(programID, name.c_str()); }
    void setUniformMat4(const std::string &name, const glm::mat4 &mat)
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    void setUniformVec3(const std::string &name, const glm::vec3 &vec)
    {
        glUniform3fv(getUniformLocation(name), 1, &vec[0]);
    }
    void setUniformVec4(const std::string &name, const glm::vec4 &vec)
    {
        glUniform4fv(getUniformLocation(name), 1, &vec[0]);
    }

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

class ComputeShaderProgram
{
public:
    GLuint programID;
    std::string programName;
    ComputeShaderProgram(const std::string &computePath, const std::string &programName) : programName(programName)
    {
        computeCode = __loadCode(computePath);
    };

    void init()
    {
        // compile compute shader
        programID = glCreateProgram();
        GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
        const char *cCode = computeCode.c_str();
        glShaderSource(computeShader, 1, &cCode, NULL);
        glCompileShader(computeShader);

        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
            std::cerr << SCRA::Utils::RED_LOG << "ComputeShaderProgram::init compute shader compilation failed\n"
                      << infoLog << SCRA::Utils::COLOR_RESET << std::endl;
        }

        glAttachShader(programID, computeShader);
        glLinkProgram(programID);
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programID, 512, NULL, infoLog);
            std::cerr << SCRA::Utils::RED_LOG << "ComputeShaderProgram::init program linking failed\n"
                      << infoLog << SCRA::Utils::COLOR_RESET << std::endl;
        }
        glDeleteShader(computeShader);
    }
    void use() { glUseProgram(programID); }
    GLuint getProgramID() { return programID; }
    GLint getUniformLocation(const std::string &name) { return glGetUniformLocation(programID, name.c_str()); }
    void setUniformMat4(const std::string &name, const glm::mat4 &mat)
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
    void setUniformVec3(const std::string &name, const glm::vec3 &vec)
    {
        glUniform3fv(getUniformLocation(name), 1, &vec[0]);
    }
    void setUniformVec4(const std::string &name, const glm::vec4 &vec)
    {
        glUniform4fv(getUniformLocation(name), 1, &vec[0]);
    }

private:
    std::string computeCode;
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
            std::cerr << "ComputeShaderProgram::__loadCode failed to read file " << path << std::endl;
        }
        // std::cerr << SCRA::Utils::GREEN_LOG << code << SCRA::Utils::COLOR_RESET << std::endl;
        return code;
    }
};