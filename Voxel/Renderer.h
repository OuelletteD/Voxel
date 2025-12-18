#pragma once
#include <glad/glad.h>
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
	void BuildChunkMesh(Chunk& chunk, const World& world, std::vector<Vertex>& solidV, std::vector<Vertex>& waterV, std::vector<unsigned int>& solidI, std::vector<unsigned int>& waterI);
	void UpdateChunk(Chunk& chunk, const World& world, const std::array<Plane, 6>& cameraPlanes);
	void RenderChunkMesh(Chunk& chunk, const World& world, const std::array<Plane, 6>& cameraPlanes, bool transparentLayer);
	void RenderWorld(World& world);
	void AddWaterSurfaceQuad(const glm::ivec3& voxelPos, std::vector<Vertex>& waterVertices, std::vector<unsigned int>& waterIndices, unsigned int& indexOffset);
	void AddQuad(const glm::vec3& center, int face, const std::array<glm::vec2, 4>& uvs, const std::array<glm::vec4, 4>& lights, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned int& indexOffset);
	void MarkChunkDirty(Chunk& chunk, const World& world);
	void UpdateMatricesUBO(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
	void Cleanup();
	void SetControls(Controls* c);
	void UpdateChunkMeshAsync(std::shared_ptr<Chunk> chunkPtr, const World& world);
private:
	Texture texture;
	Shader shader;
	Shader waterShader;
	Camera& camera;
	Controls* controls = nullptr;
	GLuint constantBuffer = {};  // OpenGL uses UBOs for storing constant data
	GLuint projectionMatrixLocation = {};  // Location for the projection matrix in the shader
	GLuint viewMatrixLocation = {};  // Location for the view matrix in the shader
	ThreadPool& threadPool;
	MainThreadDispatcher& mtd;
	void RerenderSurroundingChunks(Chunk& chunk, const World& world);
};