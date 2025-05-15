#include "Renderer.h"
#include "Debugger.h"

bool Renderer::Initialize() {
	//Initialize Texture
	bool success = texture.Initialize();
	if (!success) {
		ErrorLogger::LogError("Texture failed to initialize!");
		return false;
	}
	Texture::InitializeBlockTextures();
	texture.Bind(0); // bind to texture unit 0

	auto uvs = Texture::GetTileUVs(0, 1);
	
	// Initialize shaders
	if (!shader.Initialize("VertexShader.glsl", "FragmentShader.glsl")) {
		ErrorLogger::LogError("Failed to initialize shaders!");
		return false;
	}
	shader.Use();
	shader.SetInt("atlas", 0); // set uniform sampler to use texture unit 0
	

	Vertex vertices[] = {
		// Front face (z = -0.5) — use uvs as-is
		{ glm::vec3(-0.5f, -0.5f, -0.5f), uvs[0] },
		{ glm::vec3(0.5f, -0.5f, -0.5f), uvs[1] }, 
		{ glm::vec3(0.5f,  0.5f, -0.5f), uvs[2] },
		{ glm::vec3(-0.5f,  0.5f, -0.5f), uvs[3] },

		// Back face (z = 0.5) — flip U (swap left/right UVs)
		{ glm::vec3(-0.5f, -0.5f,  0.5f), uvs[1] },
		{ glm::vec3(0.5f, -0.5f,  0.5f), uvs[0] },
		{ glm::vec3(0.5f,  0.5f,  0.5f), uvs[3] },
		{ glm::vec3(-0.5f,  0.5f,  0.5f), uvs[2] },

		// Left face (x = -0.5) — use uvs as-is
		{ glm::vec3(-0.5f, -0.5f, -0.5f), uvs[0] },
		{ glm::vec3(-0.5f, -0.5f,  0.5f), uvs[1] },
		{ glm::vec3(-0.5f,  0.5f,  0.5f), uvs[2] },
		{ glm::vec3(-0.5f,  0.5f, -0.5f), uvs[3] },

		// Right face (x = 0.5) — use uvs as-is
		{ glm::vec3(0.5f, -0.5f, -0.5f), uvs[0] },
		{ glm::vec3(0.5f, -0.5f,  0.5f), uvs[1] },
		{ glm::vec3(0.5f,  0.5f,  0.5f), uvs[2] },
		{ glm::vec3(0.5f,  0.5f, -0.5f), uvs[3] },

		// Top face (y = 0.5) — flip V (swap top and bottom UVs)
		{ glm::vec3(-0.5f,  0.5f, -0.5f), uvs[3] }, 
		{ glm::vec3(0.5f,  0.5f, -0.5f), uvs[2] },
		{ glm::vec3(0.5f,  0.5f,  0.5f), uvs[1] },
		{ glm::vec3(-0.5f,  0.5f,  0.5f), uvs[0] }, 

		// Bottom face (y = -0.5) — flip V (swap top and bottom UVs)
		{ glm::vec3(-0.5f, -0.5f, -0.5f), uvs[1] }, 
		{ glm::vec3(0.5f, -0.5f, -0.5f), uvs[0] }, 
		{ glm::vec3(0.5f, -0.5f,  0.5f), uvs[3] },
		{ glm::vec3(-0.5f, -0.5f,  0.5f), uvs[2] }  
	};

	unsigned short indices[] = {
	0, 1, 2, 2, 3, 0,       // Front face
	4, 5, 6, 6, 7, 4,       // Back face
	8, 9, 10, 10, 11, 8,    // Left face
	12, 13, 14, 14, 15, 12, // Right face
	16, 17, 18, 18, 19, 16, // Top face
	20, 21, 22, 22, 23, 20  // Bottom face
	};

	if (!cubeMesh.Initialize(vertices, sizeof(vertices) / sizeof(Vertex), indices, sizeof(indices) / sizeof(short))) {
		ErrorLogger::LogError("Failed to initialize the cube mesh!");
		return false;
	}

	// Initialize camera
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

	glActiveTexture(GL_TEXTURE0);
	texture.Bind(0);

	BlockTextureSet tileIndex = Texture::GetBlockTexture(voxel.type);
	// Pass the model matrix to the shader
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constantBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	shader.Use();
	GLint loc = shader.GetUniformLocation("atlas");
	if (loc == -1) {
		ErrorLogger::LogError("Uniform 'atlas' not found!");
	}
	else {
		// Bind the uniform buffer to the shader program
		glUniform1i(loc, 0);  // Set the color only if the uniform location is valid
	}
	
	std::vector<Vertex> verts;
	std::vector<unsigned short> indices;
	unsigned int indexOffset = 0;

	// Cube face offsets
	const glm::vec3 faceOffsets[6][4] = {
		// Top face (y = +0.5)
		{ {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f} },
		// Bottom face (y = -0.5)
		{ {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f} },
		// Front face (z = +0.5)
		{ {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f} },
		// Back face (z = -0.5)
		{ {0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f} },
		// Left face (x = -0.5)
		{ {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f} },
		// Right face (x = +0.5)
		{ {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f} }
	};

	// Texture coordinates
	const std::array<glm::vec2, 4>* faceUVs[6] = {
		&tileIndex.top,
		&tileIndex.bottom,
		&tileIndex.side,
		&tileIndex.side,
		&tileIndex.side,
		&tileIndex.side
	};

	for (int face = 0; face < 6; ++face) {
		for (int i = 0; i < 4; ++i) {
			Vertex v;
			v.position = faceOffsets[face][i];
			v.texCoord = (*faceUVs[face])[i];
			verts.push_back(v);
		}

		// Add indices for two triangles
		indices.push_back(indexOffset + 0);
		indices.push_back(indexOffset + 1);
		indices.push_back(indexOffset + 2);

		indices.push_back(indexOffset + 2);
		indices.push_back(indexOffset + 3);
		indices.push_back(indexOffset + 0);

		indexOffset += 4;
	}

	// Build mesh and draw
	cubeMesh.SetData(verts, indices);
	cubeMesh.Upload(); // Send to GPU
	cubeMesh.Render();
}

void Renderer::SetControls(Controls* c) {
	controls = c;
}

void Renderer::Cleanup() {
	cubeMesh.Cleanup();
	glDeleteBuffers(1, &constantBuffer);
}