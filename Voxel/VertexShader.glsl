#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

out vec4 fragColor;

layout(std140) uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

void main() {
    fragColor = color;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}