#include "Renderer.h"
#include "Debugger.h"
#include "RenderingMath.h"
#include <glm/gtc/type_ptr.hpp>        // For accessing matrix data as pointers
#include "ErrorLogger.h"
#include "ChunkStatus.h"

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
	enum ChunkStatus chunkStatus = GetChunkStatus(chunk);

	if (chunkStatus != Ready && chunkStatus != Rerendering){
		if (chunkStatus == NewChunkRendering) return; //In async, waiting for return.
		ChunkPosition pos = chunk.chunkPosition;
		std::shared_ptr<Chunk> chunkPtr = world.chunks.at(pos);
		if (!world.rendered) {
			BuildChunkMesh(*chunkPtr, world);
			
			chunkPtr->chunkMesh.mesh.Upload();
			chunkPtr->chunkMesh.needsMeshUpdate = false;
			chunkPtr->chunkMesh.isUpdating = false;
			chunkPtr->chunkMesh.isNewChunk = false;
		} else {
			if (chunkStatus == NewChunk) {
				RerenderSurroundingChunks(chunk, world);
			}
			chunkPtr->chunkMesh.isUpdating = true;
			UpdateChunkMeshAsync(chunkPtr, world);
		}
		if(chunkStatus != WaitingForRerender) return; //No available mesh yet.
	}
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
	std::array<const Chunk*, 3 * 3> localChunkCache = { nullptr };
	unsigned int indexOffset = 0;
	chunk.surfaceVoxels.clear();
	chunk.surfaceVoxelGlobalPositions.clear();
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

	auto ChunkToIndex = [](int dx, int dz) -> int {
		return (dx + 1) + (dz + 1) * 3;
	};

	{
		std::shared_lock lock(world.chunkMutex);
		// Fill the cache with nearby chunks
		for (int dz = -1; dz <= 1; ++dz) {
			for (int dx = -1; dx <= 1; ++dx) {
				ChunkPosition neighborPos = chunk.chunkPosition + ChunkPosition{ dx, dz };
				auto it = world.chunks.find(neighborPos);
				if (it != world.chunks.end()) {
					localChunkCache[ChunkToIndex(dx, dz)] = it->second.get();
				}
			}
		}
	}

	auto IsVoxelSolidCached = [&](const glm::ivec3& pos) -> bool {
		if (pos.y < 0 || pos.y >= Config::CHUNK_HEIGHT) return false;

		ChunkPosition targetChunk = world.GetChunkPositionFromCoordinates(pos);
		glm::ivec2 delta = glm::ivec2(targetChunk.x - chunk.chunkPosition.x,
			targetChunk.z - chunk.chunkPosition.z);

		if (std::abs(delta.x) > 1 || std::abs(delta.y) > 1) return false;

		const Chunk* chunk = localChunkCache[ChunkToIndex(delta.x, delta.y)];
		if (!chunk) return false;

		glm::ivec3 localPos = world.ConvertPositionToPositionInsideChunk(pos);
		const Voxel* voxel = chunk->GetVoxel(localPos.x, localPos.y, localPos.z);
		return voxel && voxel->type != 0;
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
					if (!IsVoxelSolidCached(neighborPos)) {
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

	for (const glm::ivec3& posInChunk : chunk.surfaceVoxels) {
		const Voxel& voxel = chunk.voxels[posInChunk.x][posInChunk.y][posInChunk.z];
		if (voxel.type == 0) continue;
		bool grass = false;
		for (int face = 0; face < 6; ++face) {
			glm::ivec3 neighborPos = (chunk.chunkPosition * Config::CHUNK_SIZE) + posInChunk + faceDirections[face];
			if (IsVoxelSolidCached(neighborPos)) continue;
			if (face == 0 && voxel.type == 1) {
				grass = true;
			}
			const glm::vec3 voxelCenter = glm::vec3(posInChunk) + glm::vec3(0.5f);
			const std::array<glm::vec2, 4> faceUVs = GetFaceUVs(voxel, face, grass);
			for (int i = 0; i < 4; ++i) {
				Vertex v;
				v.position = faceOffsets[face][i] + voxelCenter;
				v.texCoord = faceUVs[i];

				if(Config::AO_ENABLED == true){
					float ao = calculateAOFactor(face, i, voxel.position, IsVoxelSolidCached);
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

	// Enqueue the CPU heavy mesh build on thread pool
	threadPool.enqueue([this, chunkPtr, &world]() {
		BuildChunkMesh(*chunkPtr, world);
		mtd.Enqueue([chunkPtr]() { //Pass to main thread
			chunkPtr->chunkMesh.mesh.Upload();			
			chunkPtr->chunkMesh.needsMeshUpdate = false;
			chunkPtr->chunkMesh.isUpdating = false;
			chunkPtr->chunkMesh.isNewChunk = false;
		});
	});
}

void Renderer::RerenderSurroundingChunks(Chunk& chunk, const World& world) {
	for (int x = -1; x < 2; x++) {
		for (int z = -1; z < 2; z++) {
			if (x == 0 && z == 0) continue;
			ChunkPosition chunkPosition = chunk.chunkPosition + ChunkPosition{x, z};
			std::unique_lock lock(world.chunkMutex);
			auto it = world.chunks.find(chunkPosition);
			if (it != world.chunks.end()) {
				Chunk& neighborChunk = *(it->second);
				ChunkStatus neighborStatus = GetChunkStatus(neighborChunk);
				if (neighborStatus == Ready || neighborStatus == Rerendering) {
					neighborChunk.chunkMesh.needsMeshUpdate = true;
				}
			}
		}
	}
}

void Renderer::SetControls(Controls* c) {
	controls = c;
}

void Renderer::Cleanup() {
	glDeleteBuffers(1, &constantBuffer);
}