#include "Renderer.h"
#include "Debugger.h"

bool Renderer::Initialize() {
	// Initialize shaders
	if (!shader.Initialize("VertexShader.glsl", "FragmentShader.glsl")) {
		ErrorLogger::LogError("Failed to initialize shaders!");
		return false;
	}
	shader.Use();
	shader.GetUniformLocation("voxelColor");
	Vertex vertices[] = {
		// Positions            // Colors
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Bottom-left-front (dirt brown)
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Bottom-right-front (dirt brown)
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Top-right-front (dirt brown)
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Top-left-front (dirt brown)

		{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Bottom-left-back (dirt brown)
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Bottom-right-back (dirt brown)
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Top-right-back (grass green)
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}}   // Top-left-back (grass green)
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
	camera.SetPosition(glm::vec3(-3.0f, 30.0f, -15.0f));  // Set an initial camera position
	// Create a uniform buffer object (UBO) for storing matrices (model, view, projection)
	glGenBuffers(1, &constantBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, constantBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return true;
}

void Renderer::RenderVoxel(const Voxel& voxel) {
	// Create a model matrix for the voxel (positioned correctly in world space)
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(voxel.position.x, voxel.position.y, voxel.position.z));
	glm::mat4 projection = camera.GetProjectionMatrix(800.0f / 600.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// Set the color/texture based on voxel type
	glm::vec4 voxelColor;
	switch (voxel.type) {
	case 0: // Example: air
		voxelColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Default to black for air?
		
		break;
	case 1: // Example: dirt
		voxelColor = glm::vec4(0.5f, 0.25f, 0.1f, 1.0f); // Dirt brown color
		
		break;
		// Add more voxel types (e.g., water, stone) as needed
	case 2: // Example: grass
		voxelColor = glm::vec4(0.13f, 0.55f, 0.13f, 1.0f); // Grass green color
	default:
		
		break;
	}
	// Pass the model matrix to the shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constantBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	shader.Use();
	GLint loc = shader.GetUniformLocation("voxelColor");
	if (loc == -1) {
		std::cout << "Uniform 'voxelColor' not found!" << std::endl;
	}
	else {
		glUniform4fv(loc, 1, glm::value_ptr(voxelColor));  // Set the color only if the uniform location is valid
	}
	// Bind the uniform buffer to the shader program
	
	cubeMesh.Render();
}

void Renderer::Cleanup() {
	cubeMesh.Cleanup();
	glDeleteBuffers(1, &constantBuffer);
}

