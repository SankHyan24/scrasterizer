#version 430 core

layout (local_size_x = 16, local_size_y = 16) in; 

layout (binding = 0) writeonly uniform image2D imgOutput; 

layout(binding = 1) buffer DataBuffer {
    float data[]; 
};


void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy); 

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    if (coord.x < 800 && coord.y < 800) {
        int index = coord.y * 800 + coord.x; 
        float value = data[index]; 
        color = vec4(value, value, value, 1.0); 
    }

    imageStore(imgOutput, coord, color);
}