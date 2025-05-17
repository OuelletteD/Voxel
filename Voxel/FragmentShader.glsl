#version 330 core

//uniform to pass it from glUniform4fv
// 'in' is used to receive data from the vertex shader
in vec2 TexCoord;
in vec4 vLight;
uniform sampler2D atlas;
out vec4 FragColor;     // 'out' is used to output the final color to the screen

void main() {
    FragColor = texture(atlas, TexCoord) * vLight;  // Output the interpolated color from the vertex shader
}