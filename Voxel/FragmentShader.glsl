#version 330 core

in vec4 fragColor;  // 'in' is used to receive data from the vertex shader
out vec4 color;     // 'out' is used to output the final color to the screen

void main() {
    color = fragColor;  // Output the interpolated color from the vertex shader
}