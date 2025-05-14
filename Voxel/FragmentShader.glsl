#version 330 core

uniform vec4 voxelColor;  //uniform to pass it from glUniform4fv
// 'in' is used to receive data from the vertex shader
out vec4 color;     // 'out' is used to output the final color to the screen

void main() {
    color = voxelColor;  // Output the interpolated color from the vertex shader
}