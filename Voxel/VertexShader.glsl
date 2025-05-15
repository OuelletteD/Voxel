#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord; // Vertex texture coordinate

layout(std140) uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
};

out vec2 TexCoord;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPosition, 1.0);
    TexCoord = aTexCoord;
}