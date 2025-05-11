#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"

class Renderer {
public:
	bool Initialize();
	void Render();
	void Cleanup();
private:
	Shader shader;
	Mesh cubeMesh;
	Camera camera;
	GLuint constantBuffer;  // OpenGL uses UBOs for storing constant data
	GLuint projectionMatrixLocation;  // Location for the projection matrix in the shader
	GLuint viewMatrixLocation;  // Location for the view matrix in the shader
};