#version 430 core

uniform sampler2D imgOutput;

in vec2 TexCoords;
in vec3 ourColor; 
out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0); 
    FragColor = texture(imgOutput, TexCoords);
}