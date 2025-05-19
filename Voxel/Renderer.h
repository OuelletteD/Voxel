#pragma once
#include <GL/glew.h>
#include "Camera.h"
#include "Controls.h"
#include "Mesh.h"
#include "Shader.h"
#include "World.h"
#include "Texture.h"

class Renderer {
public:
	Renderer(Camera& cam) : camera(cam), texture("Terrain.png") {}
	bool Initialize();
	void RenderChunk(Chunk& chunk, const World& world);
	void BuildChunkMesh(Chunk& chunk, const World& world);
	void RenderWorld(World& world);
	void Cleanup();
	void SetControls(Controls* c);
private:
	Texture texture;
	Shader shader;
	Camera& camera;
	Controls* controls = nullptr;
	GLuint constantBuffer = {};  // OpenGL uses UBOs for storing constant data
	GLuint projectionMatrixLocation = {};  // Location for the projection matrix in the shader
	GLuint viewMatrixLocation = {};  // Location for the view matrix in the shader
};