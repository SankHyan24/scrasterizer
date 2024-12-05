#version 430 core

layout (local_size_x = 16, local_size_y = 16) in; 

layout (binding = 0) writeonly uniform image2D imgOutput; 

void main() {
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy); 

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    if (coord.y % 2 == 0) {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        color = vec4(0.0, 0.0, 1.0, 1.0);
    }

    imageStore(imgOutput, coord, color);
}