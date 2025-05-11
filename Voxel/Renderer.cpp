#include "Renderer.h"
#include "Debugger.h"

bool Renderer::Initialize() {
	// Initialize shaders
	if (!shader.Initialize("VertexShader.glsl", "FragmentShader.glsl")) {
		ErrorLogger::LogError("Failed to initialize shaders!");
		return false;
	}
	Vertex vertices[] = {
		// Positions            // Colors
		{{-0.5f, -0.5f, -0.5f}, {0.55f, 0.27f, 0.07f, 1.0f}},  // Bottom-left-front (dirt brown)
		{{ 0.5f, -0.5f, -0.5f}, {0.55f, 0.27f, 0.07f, 1.0f}},  // Bottom-right-front (dirt brown)
		{{ 0.5f,  0.5f, -0.5f}, {0.55f, 0.27f, 0.07f, 1.0f}},  // Top-right-front (dirt brown)
		{{-0.5f,  0.5f, -0.5f}, {0.55f, 0.27f, 0.07f, 1.0f}},  // Top-left-front (dirt brown)

		{{-0.5f, -0.5f,  0.5f}, {0.55f, 0.27f, 0.07f, 1.0f}},  // Bottom-left-back (dirt brown)
		{{ 0.5f, -0.5f,  0.5f}, {0.55f, 0.27f, 0.07f, 1.0f}},  // Bottom-right-back (dirt brown)
		{{ 0.5f,  0.5f,  0.5f}, {0.49f, 0.99f, 0.0f, 1.0f}},  // Top-right-back (grass green)
		{{-0.5f,  0.5f,  0.5f}, {0.49f, 0.99f, 0.0f, 1.0f}}   // Top-left-back (grass green)
	};

	unsigned short indices[] = {
		0, 1, 2, 2, 3, 0,  // Front face
		4, 5, 6, 6, 7, 4,  // Back face
		0, 1, 5, 5, 4, 0,  // Bottom face
		2, 3, 7, 7, 6, 2,  // Top face
		0, 3, 7, 7, 4, 0,  // Left face
		1, 2, 6, 6, 5, 1   // Right face
	};

	if (!cubeMesh.Initialize(vertices, sizeof(vertices) / sizeof(Vertex), indices, sizeof(indices) / sizeof(short))) {
		ErrorLogger::LogError("Failed to initialize the cube mesh!");
		return false;
	}

	// Initialize camera
	camera.SetPosition(glm::vec3(2.0f, 3.0f, -2.0f));  // Set an initial camera position
	// Create a uniform buffer object (UBO) for storing matrices (model, view, projection)
	glGenBuffers(1, &constantBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, constantBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return true;
}

void Renderer::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear the color and depth buffers
	shader.Use();
	// Update the camera and projection matrices
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);  // Identity matrix for the model (no transformation)

	glBindBuffer(GL_UNIFORM_BUFFER, constantBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the uniform buffer to the shader program
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constantBuffer);
	// Render the cube mesh
	cubeMesh.Render();
}

void Renderer::Cleanup() {
	cubeMesh.Cleanup();
	glDeleteBuffers(1, &constantBuffer);
}

