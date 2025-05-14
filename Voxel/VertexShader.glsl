#version 330 core
layout(location = 0) in vec3 position;

layout(std140) uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}