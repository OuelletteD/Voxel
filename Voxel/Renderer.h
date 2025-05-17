#pragma once
#include <GL/glew.h>
#include "Camera.h"
#include "Controls.h"
#include "Mesh.h"
#include "Shader.h"
#include "World.h"
#include "Texture.h"

const glm::ivec3 faceDirections[6] = {
	{0, 1, 0},   // Top
	{0, -1, 0},  // Bottom
	{0, 0, 1},   // Front
	{0, 0, -1},  // Back
	{-1, 0, 0},  // Left
	{1, 0, 0}    // Right
};

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