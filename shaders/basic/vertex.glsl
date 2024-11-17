#version 330 core

layout(location = 0) in vec3 aPos;
uniform mat4 view; // 视图矩阵
uniform mat4 projection; // 投影矩阵
uniform mat4 model; // 模型矩阵
out vec3 ourColor;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    // calc normal
    
    ourColor = aPos;
}