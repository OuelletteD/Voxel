#pragma once
#include <GL/glew.h>
#include "ErrorLogger.h"
#include <glm.hpp>        // Core GLM functionality
#include <gtc/matrix_transform.hpp> // Transformations like translation, rotation
#include <gtc/type_ptr.hpp>        // For accessing matrix data as pointers
#include <vector>

struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoord;
};

class Mesh {
public:
	GLuint vao = 0;  // Vertex Array Object
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	unsigned int indexCount = 0;

	std::vector<Vertex> localVertices;
	std::vector<unsigned short> localIndices;

	bool Initialize(const Vertex* vertices, unsigned int vertexCount, const unsigned short* indices, unsigned int indexCount);
	void Render();
	void Cleanup();

	void SetData(const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices);
	void Upload();
};
