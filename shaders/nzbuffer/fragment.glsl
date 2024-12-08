#version 450 core

uniform sampler2D imgOutput;

in vec2 TexCoords;
in vec3 ourColor; 
out vec4 FragColor;

void main() {
    FragColor = texture(imgOutput, TexCoords); // use the color from the texture
    FragColor = vec4(ourColor, 1.0); 
}