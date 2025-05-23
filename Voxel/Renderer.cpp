#include "Renderer.h"
#include "Debugger.h"
#include "RenderingMath.h"
#include <glm/gtc/type_ptr.hpp>        // For accessing matrix data as pointers
#include "ErrorLogger.h"

bool Renderer::Initialize() {
	if (!texture.Initialize()) {
		ErrorLogger::LogError("Texture failed to initialize!");
		return false;
	}
	Texture::InitializeBlockTextures();
	texture.Bind(0);
	auto uvs = Texture::GetTileUVs(0, 1);
	
	if (!shader.Initialize("VertexShader.glsl", "FragmentShader.glsl")) {
		ErrorLogger::LogError("Failed to initialize shaders!");
		return false;
	}
	shader.Use();
	shader.SetInt("atlas", 0);

	// Create a uniform buffer object (UBO) for storing matrices (model, view, projection)
	glGenBuffers(1, &constantBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, constantBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return true;
}

void Renderer::RenderChunk(Chunk& chunk, const World& world, const std::array<Plane,6>& cameraPlanes) {
	if (chunk.chunkMesh.needsMeshUpdate) {
		chunk.surfaceVoxels.clear();
		chunk.surfaceVoxelGlobalPositions.clear();
		ChunkPosition pos = chunk.chunkPosition;
		std::shared_ptr<Chunk> chunkPtr = world.chunks.at(pos);

		if (!world.rendered) {
			chunkPtr->chunkMesh.needsMeshUpdate = false;
			BuildChunkMesh(*chunkPtr, world);
			chunkPtr->chunkMesh.mesh.Upload();
			chunkPtr->chunkMesh.isUploaded = true;
		} else {
			UpdateChunkMeshAsync(chunkPtr, world);
		}
	}
	if (!chunk.chunkMesh.isUploaded) return;
	glm::vec3 worldChunkPosition = {
		chunk.chunkPosition.x * Config::CHUNK_SIZE,
		0.0f,
		chunk.chunkPosition.z * Config::CHUNK_SIZE
	};
	glm::vec3 minBound = worldChunkPosition;
	glm::vec3 maxBound = glm::vec3(worldChunkPosition.x + Config::CHUNK_SIZE, Config::CHUNK_HEIGHT, worldChunkPosition.z + Config::CHUNK_SIZE);
	if (!IsChunkInFrustum(cameraPlanes, minBound, maxBound)) {
		return;
	}

	// Create a model matrix for the voxel (positioned correctly in world space)
	glm::mat4 model = glm::translate(glm::mat4(1.0f), worldChunkPosition);
	glm::mat4 projection = camera.GetProjectionMatrix();
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
	} else {
		glUniform1i(loc, 0);
	}
	chunk.chunkMesh.mesh.Render();
}

void Renderer::BuildChunkMesh(Chunk& chunk, const World& world) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int indexOffset = 0;
	auto isSolidAt = [&](glm::ivec3 pos) -> bool {
		return world.IsVoxelSolidAt(pos);
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
		for (int y = 0; y < Config::CHUNK_HEIGHT; y++) {
			for (int z = 0; z < Config::CHUNK_SIZE; z++) {
				const Voxel& voxel = chunk.voxels[x][y][z];
				if (voxel.type == 0) continue;
				bool isSurface = false;
				glm::ivec3 posInChunk = { x,y,z };
				for (int face = 0; face < 6; ++face) {
					glm::ivec3 neighborPos = (chunk.chunkPosition * Config::CHUNK_SIZE) + posInChunk + faceDirections[face];
					bool grass = false;
					if (!world.IsVoxelSolidAt(neighborPos)) {
						isSurface = true;
						break;
					}
				}
				if (isSurface) {
					glm::ivec3 globalVoxelPosition = { x + chunk.chunkPosition.x * Config::CHUNK_SIZE, y, z + chunk.chunkPosition.z * Config::CHUNK_SIZE };
					chunk.surfaceVoxels.push_back(posInChunk);
					chunk.surfaceVoxelGlobalPositions.push_back(globalVoxelPosition);
				}		
			}
		}
	}
	std::unordered_map<glm::ivec3, bool, ivec3_hash> lightingMap;
	if (Config::SUNLIGHT_ON) {
		std::unordered_map<glm::ivec3, bool, ivec3_hash> lightingMap = CalculateLighting(world, chunk, isSolidAt);
	}

	for (const glm::ivec3& posInChunk : chunk.surfaceVoxels) {
		const Voxel& voxel = chunk.voxels[posInChunk.x][posInChunk.y][posInChunk.z];
		if (voxel.type == 0) continue;
		bool grass = false;
		for (int face = 0; face < 6; ++face) {
			glm::ivec3 neighborPos = (chunk.chunkPosition * Config::CHUNK_SIZE) + posInChunk + faceDirections[face];
			if (world.IsVoxelSolidAt(neighborPos)) continue;
			if (face == 0 && voxel.type == 1) {
				grass = true;
			}
			const glm::vec3 voxelCenter = glm::vec3(posInChunk) + glm::vec3(0.5f);
			const std::array<glm::vec2, 4> faceUVs = GetFaceUVs(voxel, face, grass);
			for (int i = 0; i < 4; ++i) {
				Vertex v;
				v.position = faceOffsets[face][i] + voxelCenter;
				v.texCoord = faceUVs[i];

				if (Config::SUNLIGHT_ON) {
					v.light = lightingMap[voxel.position] ? glm::vec4(1.0f) : glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
					
				} else if(Config::AO_ENABLED == true){
					float ao = calculateAOFactor(face, i, voxel.position, isSolidAt);
					v.light = glm::vec4(ao, ao, ao, 1.0f);
				} else {
					//No lighting
					v.light = glm::vec4(1.0f);
				}

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

	chunk.chunkMesh.mesh.SetData(vertices, indices);
}

void Renderer::RenderWorld(World& world) {
	std::array<Plane, 6> cameraPlanes = camera.ExtractFrustumPlanes();
	for (auto& [chunkPos, chunk] : world.chunks) {
		RenderChunk(*chunk, world, cameraPlanes);
	}
	if (!world.rendered) world.rendered = true;
}

void Renderer::UpdateChunkMeshAsync(std::shared_ptr<Chunk> chunkPtr, const World& world) {
	chunkPtr->chunkMesh.needsMeshUpdate = false;

	// Enqueue the CPU heavy mesh build on thread pool
	threadPool.enqueue([this, chunkPtr, &world]() {
		BuildChunkMesh(*chunkPtr, world);
		mtd.Enqueue([chunkPtr]() { //Pass to main thread
			chunkPtr->chunkMesh.mesh.Upload();
			chunkPtr->chunkMesh.isUploaded = true;
		});
	});
}

void Renderer::SetControls(Controls* c) {
	controls = c;
}

void Renderer::Cleanup() {
	glDeleteBuffers(1, &constantBuffer);
}