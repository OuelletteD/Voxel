#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Camera.h"
#include "Controls.h"
#include "Mesh.h"
#include "Shader.h"
#include "World.h"

struct VoxelRender {
	glm::mat4 modelMatrix;
	glm::vec4 color;
};

class Renderer {
public:
	bool Initialize();
	void RenderVoxel(const Voxel& voxel);
	void Cleanup();
	void SetControls(Controls* c);
private:
	Shader shader;
	Mesh cubeMesh;
	Camera camera;
	Controls* controls = nullptr;
	GLuint constantBuffer;  // OpenGL uses UBOs for storing constant data
	GLuint projectionMatrixLocation;  // Location for the projection matrix in the shader
	GLuint viewMatrixLocation;  // Location for the view matrix in the shader
};