#pragma once
#include <GL/glew.h>
#include "ErrorLogger.h"
#include <glm/glm.hpp>        // Core GLM functionality
#include <glm/gtc/matrix_transform.hpp> // Transformations like translation, rotation
#include <glm/gtc/type_ptr.hpp>        // For accessing matrix data as pointers
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec4 light;
	//glm::vec3 normal;
};

class Mesh {
public:
	Mesh() = default;
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// enable move
	Mesh(Mesh&&) = default;
	Mesh& operator=(Mesh&&) = default;
	GLuint vao = 0;  // Vertex Array Object
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	unsigned int indexCount = 0;

	std::vector<Vertex> localVertices;
	std::vector<unsigned int> localIndices;

	bool Initialize(const Vertex* vertices, unsigned int vertexCount, const unsigned int* indices, unsigned int indexCount);
	void Render();
	void Cleanup();

	void SetData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	void Upload();
};

struct ChunkMesh {
	Mesh mesh;
	bool needsMeshUpdate = true;

	ChunkMesh() = default;

	ChunkMesh(const ChunkMesh&) = delete;
	ChunkMesh& operator=(const ChunkMesh&) = delete;

	ChunkMesh(ChunkMesh&&) = default;
	ChunkMesh& operator=(ChunkMesh&&) = default;
};
