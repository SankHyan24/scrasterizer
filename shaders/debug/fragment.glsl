#version 430 core

uniform sampler2D imgOutput;

in vec2 TexCoords;
in vec3 ourColor; 
out vec4 FragColor;

// void main() {
//     FragColor = texture(imgOutput, TexCoords);
//     FragColor = vec4(ourColor, 1.0); 
// }

void main() {
    // 从 imgOutput 纹理中读取对应片元的 Z 值
    float zFromTexture = texture(imgOutput, TexCoords).r;

    // 如果需要手动进行深度比较：
    // if (gl_FragCoord.z > zFromTexture) {
    //     discard; // 当前片元被遮挡，丢弃
    // }

    // 使用颜色值 ourColor 进行片元着色
    FragColor = vec4(ourColor, 1.0); // 输出最终颜色
    FragColor = texture(imgOutput, TexCoords);
}