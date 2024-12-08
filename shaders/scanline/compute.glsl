#version 450 core

layout (local_size_x = 16, local_size_y = 16) in; 
layout (binding = 0) writeonly uniform image2D imgOutput; 
layout(binding = 1) buffer DataBuffer {
    float data[]; 
};

void main() 
{
    // coord+1
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    if (coord.x < 800 && coord.y < 800) {// mapping from float to rgba
        int index = coord.y * 800 + coord.x; 
        float value = data[index]; 
        // unpack the float value to 4 bytes
        uint encodedInt = floatBitsToInt(value);
        uint byte3 = (encodedInt >> 24) & 0xFF;  
        uint byte2 = (encodedInt >> 16) & 0xFF;  
        uint byte1 = (encodedInt >> 8) & 0xFF;
        uint byte0 = encodedInt & 0xFF;   
        // then pack the 4 bytes to a vec4
        float r = float(byte0) / 255.0;
        float g = float(byte1) / 255.0;
        float b = float(byte2) / 255.0;
        float a = float(byte3) / 255.0;
        color = vec4(r, g, b, a); 
    }
    imageStore(imgOutput, coord, color);
}