#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Debugger.h"
#include "ErrorLogger.h"
#include <glm.hpp>        // Core GLM functionality
#include <gtc/matrix_transform.hpp> // Transformations like translation, rotation
#include <gtc/type_ptr.hpp>        // For accessing matrix data as pointers

struct Vertex {
	GLfloat position[3];
	GLfloat color[4];
};

class Mesh {
public:
	GLuint vao;  // Vertex Array Object
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	unsigned int indexCount;

	bool Initialize(const Vertex* vertices, unsigned int vertexCount, const unsigned short* indices, unsigned int indexCount);
	void Render();
	void Cleanup();
};
