#version 330 core

//uniform to pass it from glUniform4fv
// 'in' is used to receive data from the vertex shader
in vec2 TexCoord;
in vec4 vLight;
uniform sampler2D atlas;
uniform float uAlpha;
out vec4 FragColor;     // 'out' is used to output the final color to the screen

void main() {
    vec4 tex = texture(atlas, TexCoord);
    vec4 color = tex * vLight;
    color.a *= uAlpha;
    FragColor = color;  // Output the interpolated color from the vertex shader
}