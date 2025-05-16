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

	unsigned int indices[] = {
	0, 1, 2, 2, 3, 0,       // Front face
	4, 5, 6, 6, 7, 4,       // Back face
	8, 9, 10, 10, 11, 8,    // Left face
	12, 13, 14, 14, 15, 12, // Right face
	16, 17, 18, 18, 19, 16, // Top face
	20, 21, 22, 22, 23, 20  // Bottom face
	};

	if (!chunkMesh.Initialize(vertices, sizeof(vertices) / sizeof(Vertex), indices, sizeof(indices) / sizeof(short))) {
		ErrorLogger::LogError("Failed to initialize the chunk mesh!");
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

void Renderer::RenderChunk(const Chunk& chunk, const World& world) {
	BuildChunkMesh(chunk, world);
	chunkMesh.Upload(); // Send to GPU

	glm::vec3 worldChunkPosition = {
		chunk.chunkPosition.x * Config::CHUNK_SIZE,
		0.0f,
		chunk.chunkPosition.z * Config::CHUNK_SIZE
	};

	// Create a model matrix for the voxel (positioned correctly in world space)
	glm::mat4 model = glm::translate(glm::mat4(1.0f), worldChunkPosition);
	glm::mat4 projection = camera.GetProjectionMatrix(Config::SCREEN_WIDTH / Config::SCREEN_HEIGHT);
	glm::mat4 view = camera.GetViewMatrix();

	glActiveTexture(GL_TEXTURE0);
	texture.Bind(0);
	
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
	chunkMesh.Render();

}

void Renderer::BuildChunkMesh(const Chunk& chunk, const World& world) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
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
	auto GetFaceUVs = [](const Voxel& voxel, int face, bool grass) -> const std::array<glm::vec2, 4>{
		BlockTextureSet tileIndex = Texture::GetBlockTexture(voxel.type);
		if(grass) tileIndex = Texture::GetBlockTexture(2);
		switch (face) {
			case 0: return tileIndex.top;
			case 1: return tileIndex.bottom;
			default: return tileIndex.side;
		}
	};
	
	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int y = 0; y < Config::CHUNK_SIZE; y++) {
			for (int z = 0; z < Config::CHUNK_SIZE; z++) {
				const Voxel& voxel = chunk.voxels[x][y][z];
				
				if (voxel.type == 0) continue;
				glm::ivec3 posInChunk = { x,y,z };
				for (int face = 0; face < 6; ++face) {
					glm::ivec3 neighborPos = (chunk.chunkPosition * Config::CHUNK_SIZE) + posInChunk + faceDirections[face];
					bool grass = false;
					if (world.IsVoxelSolidAt(neighborPos)) {
						continue;
					}
					if (face == 0 && voxel.type == 1) {
						grass = true;
					}

					const glm::vec3 voxelCenter = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);
					const std::array<glm::vec2, 4> faceUVs = GetFaceUVs(voxel, face, grass);
					for (int i = 0; i < 4; ++i) {
						Vertex v;
						v.position = faceOffsets[face][i] + voxelCenter;
						v.texCoord = faceUVs[i];
						
						vertices.push_back(v);
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
			}
		}
	}
	chunkMesh.SetData(vertices, indices);
}

void Renderer::RenderWorld(const World& world) {
	for (const auto& [chunkPos, chunk] : world.chunks) {
		RenderChunk(chunk, world);
	}
}

void Renderer::SetControls(Controls* c) {
	controls = c;
}

void Renderer::Cleanup() {
	chunkMesh.Cleanup();
	glDeleteBuffers(1, &constantBuffer);
}