#pragma once
#include <GL/glew.h>
#include "Camera.h"
#include "Controls.h"
#include "Shader.h"
#include "World.h"
#include "Texture.h"
#include "MainThreadDispatcher.h"

class Renderer {
public:
	Renderer(Camera& cam, ThreadPool& threadPool, MainThreadDispatcher& mtd) : camera(cam), threadPool(threadPool), mtd(mtd), texture("Terrain.png") {}
	bool Initialize();
	void RenderChunk(Chunk& chunk, const World& world, const std::array<Plane, 6>& cameraPlanes);
	void BuildChunkMesh(Chunk& chunk, const World& world);
	void RenderWorld(World& world);
	void Cleanup();
	void SetControls(Controls* c);
	void UpdateChunkMeshAsync(std::shared_ptr<Chunk> chunkPtr, const World& world);
private:
	Texture texture;
	Shader shader;
	Camera& camera;
	Controls* controls = nullptr;
	GLuint constantBuffer = {};  // OpenGL uses UBOs for storing constant data
	GLuint projectionMatrixLocation = {};  // Location for the projection matrix in the shader
	GLuint viewMatrixLocation = {};  // Location for the view matrix in the shader
	ThreadPool& threadPool;
	MainThreadDispatcher& mtd;
};