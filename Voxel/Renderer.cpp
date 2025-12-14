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
		std::shared_ptr<Chunk> chunkPtr;
		{
			std::unique_lock lock(world.chunkMutex);
			auto it = world.chunks.find(chunk.chunkPosition);
			if (it == world.chunks.end()) return;
			chunkPtr = it->second;
		}
		
		if (!world.rendered) {
			std::vector<Vertex> solidV, waterV;
			std::vector<unsigned int> solidI, waterI;
			BuildChunkMesh(*chunkPtr, world, solidV, waterV, solidI, waterI);
			chunkPtr->chunkMesh.mesh.SwapCPUData(solidV, solidI);
			chunkPtr->chunkMesh.mesh.Upload();
			if (!waterV.empty() && !waterI.empty()) {
				chunkPtr->waterMesh.mesh.SwapCPUData(waterV, waterI);
			}
			chunkPtr->waterMesh.mesh.Upload();

			chunkPtr->chunkMesh.dirty.store(false, std::memory_order_relaxed);
			chunkPtr->chunkMesh.queued.store(false, std::memory_order_relaxed);
			chunkPtr->chunkMesh.isNewChunk = false;
			return;
		}
		if (chunkStatus == NewChunk) {
			RerenderSurroundingChunks(chunk, world);
		}
		if (chunkPtr->chunkMesh.dirty.load(std::memory_order_acquire)) {
			MarkChunkDirty(*chunkPtr, world);
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
		ErrorLogger::LogError("Uniform 'atlas' not found in chunk shader!");
	} else {
		glUniform1i(loc, 0);
	}
	if (!chunk.chunkMesh.mesh.IsEmpty()) {
		chunk.chunkMesh.mesh.Render();
	}
	if (!chunk.waterMesh.mesh.IsEmpty()) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		shader.Use();
		GLint waterAatlasLoc = shader.GetUniformLocation("atlas");
		if (waterAatlasLoc == -1) {
			ErrorLogger::LogError("Uniform 'atlas' not found in water shader!");
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			texture.Bind(0);
			glUniform1i(waterAatlasLoc, 0);
		}

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, constantBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(model));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(projection));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		chunk.waterMesh.mesh.Render();

		glDisable(GL_BLEND);
	}
}

void Renderer::BuildChunkMesh(Chunk& chunk, const World& world, std::vector<Vertex>& solidV, std::vector<Vertex>& waterV, std::vector<unsigned int>& solidI, std::vector<unsigned int>& waterI) {
	std::array<std::shared_ptr<Chunk>, 3 * 3> localChunkCache = { nullptr };
	unsigned int indexOffset = 0;
	chunk.surfaceVoxels.clear();
	chunk.surfaceVoxelGlobalPositions.clear();
	// Texture coordinates
	auto GetFaceUVs = [](const Voxel& voxel, int face, bool grass) -> const std::array<glm::vec2, 4>{
		BlockTextureSet tileIndex = Texture::GetBlockTexture(voxel.type);
		if(grass) tileIndex = Texture::GetBlockTexture(BlockType::Grass);
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
					localChunkCache[ChunkToIndex(dx, dz)] = it->second;
				}
			}
		}
	}

	auto IsVoxelSolidCached = [&](const glm::ivec3& pos) -> BlockType {
		if (pos.y < 0 || pos.y >= Config::CHUNK_HEIGHT) return BlockType::Air;

		ChunkPosition targetChunk = world.GetChunkPositionFromCoordinates(pos);
		glm::ivec2 delta = glm::ivec2(targetChunk.x - chunk.chunkPosition.x,
			targetChunk.z - chunk.chunkPosition.z);

		if (std::abs(delta.x) > 1 || std::abs(delta.y) > 1) return BlockType::Air;

		const std::shared_ptr<Chunk> chunk = localChunkCache[ChunkToIndex(delta.x, delta.y)];
		if (!chunk) return BlockType::Air;

		glm::ivec3 localPos = world.ConvertPositionToPositionInsideChunk(pos);
		const Voxel* voxel = chunk->GetVoxel(localPos.x, localPos.y, localPos.z);
		if (!voxel) return BlockType::Air;
		return voxel->type;
	};
	
	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int y = 0; y < Config::CHUNK_HEIGHT; y++) {
			for (int z = 0; z < Config::CHUNK_SIZE; z++) {
				const Voxel& voxel = chunk.voxels[x][y][z];
				if (voxel.type == BlockType::Air) continue;
				bool isSurface = false;
				glm::ivec3 posInChunk = { x,y,z };
				for (int face = 0; face < 6; ++face) {
					glm::ivec3 neighborPos = (chunk.chunkPosition * Config::CHUNK_SIZE) + posInChunk + faceDirections[face];
					bool grass = false;
					int neighborType = IsVoxelSolidCached(neighborPos);
					if (neighborType == BlockType::Air || neighborType == BlockType::Water) {
						isSurface = true;
						break;
					}
				}
				if (isSurface) {
					glm::ivec3 globalVoxelPosition = { x + chunk.chunkPosition.x * Config::CHUNK_SIZE, y, z + chunk.chunkPosition.z * Config::CHUNK_SIZE };
					std::lock_guard<std::mutex> lock(chunk.meshMutex);
					chunk.surfaceVoxels.push_back(posInChunk);
					chunk.surfaceVoxelGlobalPositions.push_back(globalVoxelPosition);
				}		
			}
		}
	}
	unsigned int waterIndexOffset = 0;
	chunk.waterMesh.mesh.localVertices.clear();
	chunk.waterMesh.mesh.localIndices.clear();
	std::unordered_map<glm::ivec3, bool, ivec3_hash> lightingMap;
	std::lock_guard<std::mutex> lock(chunk.meshMutex);
	for (const glm::ivec3& posInChunk : chunk.surfaceVoxels) {
		if (posInChunk.y < 0 || posInChunk.y >= Config::CHUNK_HEIGHT) {
			continue;
		}
		const Voxel& voxel = chunk.voxels[posInChunk.x][posInChunk.y][posInChunk.z];
		if (voxel.type == BlockType::Water) {
			glm::ivec3 neighborPos = (chunk.chunkPosition * Config::CHUNK_SIZE) + posInChunk + faceDirections[0];
			if (IsVoxelSolidCached(neighborPos) != BlockType::Air) {
				continue;
			}
			AddWaterSurfaceQuad(posInChunk, waterV, waterI, waterIndexOffset);
			continue;
		}
		if (voxel.type == BlockType::Air) continue;
		bool grass = false;
		for (int face = 0; face < 6; ++face) {
			glm::ivec3 neighborPos = (chunk.chunkPosition * Config::CHUNK_SIZE) + posInChunk + faceDirections[face];
			int neighborType = IsVoxelSolidCached(neighborPos);
			if (neighborType != BlockType::Air && neighborType != BlockType::Water) continue;
			if (face == 0 && voxel.type == BlockType::Dirt && neighborType != BlockType::Water) {
				grass = true;
			}
			const glm::vec3 voxelCenter = glm::vec3(posInChunk) + glm::vec3(0.5f);
			const std::array<glm::vec2, 4> faceUVs = GetFaceUVs(voxel, face, grass);
			std::array<glm::vec4, 4> lights;

			if (Config::AO_ENABLED) {
				for (int i = 0; i < 4; ++i) {
					float ao = calculateAOFactor(face, i, voxel.position, IsVoxelSolidCached);
					lights[i] = glm::vec4(ao, ao, ao, 1.0f);
				}
			}else {
				lights.fill(glm::vec4(1.0f));
			}
			AddQuad(voxelCenter, face, faceUVs, lights, solidV, solidI, indexOffset);
		}
	}
}

void Renderer::AddWaterSurfaceQuad(const glm::ivec3& voxelPos, std::vector<Vertex>& waterVertices, std::vector<unsigned int>& waterIndices, unsigned int& indexOffset) {
	glm::vec3 center = glm::vec3(voxelPos) + glm::vec3(0.5f, 0.0f, 0.5f);

	std::array<glm::vec2, 4> waterUVs = texture.GetTileUVs(13, 0);
	std::array<glm::vec4, 4> waterLights;
	waterLights.fill(glm::vec4(1.0f));
	AddQuad(center, 0, waterUVs, waterLights, waterVertices, waterIndices, indexOffset);
}

void Renderer::AddQuad(const glm::vec3& center, int face, const std::array<glm::vec2, 4>& uvs, const std::array<glm::vec4, 4>& lights, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, unsigned int& indexOffset) {
	for (int i = 0; i < 4; i++) {
		Vertex v;
		v.position = center + faceOffsets[face][i];
		v.texCoord = uvs[i];
		v.light = lights[i];
		vertices.push_back(v);
	}

	indices.push_back(indexOffset + 0);
	indices.push_back(indexOffset + 1);
	indices.push_back(indexOffset + 2);

	indices.push_back(indexOffset + 2);
	indices.push_back(indexOffset + 3);
	indices.push_back(indexOffset + 0);

	indexOffset += 4;
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
	chunkPtr->chunkMesh.queued.store(true, std::memory_order_relaxed);
	threadPool.enqueue([this, chunkPtr, &world]() {
		chunkPtr->chunkMesh.building.store(true, std::memory_order_release);
		chunkPtr->chunkMesh.dirty.store(false, std::memory_order_release);
		std::vector<Vertex> solidV, waterV;
		std::vector<unsigned int> solidI, waterI;
		BuildChunkMesh(*chunkPtr, world, solidV, waterV, solidI, waterI);
		auto svLocal = std::move(solidV);
		auto siLocal = std::move(solidI);
		auto wvLocal = std::move(waterV);
		auto wiLocal = std::move(waterI);
		mtd.Enqueue([this, chunkPtr, &world, sv = std::move(svLocal), si = std::move(siLocal), wv = std::move(wvLocal), wi = std::move(wiLocal)]() mutable { //Pass to main thread
			if (!sv.empty() && !si.empty()) {
				chunkPtr->chunkMesh.mesh.SwapCPUData(sv, si);
			}
			
			if (!wv.empty() && !wi.empty()) {
				chunkPtr->waterMesh.mesh.SwapCPUData(wv, wi);
			}
			chunkPtr->chunkMesh.mesh.Upload();
			chunkPtr->waterMesh.mesh.Upload();
			
			
			chunkPtr->chunkMesh.building.store(false, std::memory_order_release);
			chunkPtr->chunkMesh.queued.store(false, std::memory_order_release);
			chunkPtr->chunkMesh.isNewChunk = false;
			if (chunkPtr->chunkMesh.dirty.load(std::memory_order_acquire)) {
				this->MarkChunkDirty(*chunkPtr, world);
			}
		});
	});
}

void Renderer::RerenderSurroundingChunks(Chunk& chunk, const World& world) {
	for (int x = -1; x < 2; x++) {
		for (int z = -1; z < 2; z++) {
			if (x == 0 && z == 0) continue;
			ChunkPosition chunkPosition = chunk.chunkPosition + ChunkPosition{x, z};
			std::shared_ptr<Chunk> neighborChunkPtr;
			{
				std::unique_lock lock(world.chunkMutex);
				auto it = world.chunks.find(chunkPosition);
				if (it == world.chunks.end()) continue;
				neighborChunkPtr = it->second;
			}
			ChunkStatus neighborStatus = GetChunkStatus(*neighborChunkPtr);
			if (neighborStatus == Ready || neighborStatus == Rerendering) {
				MarkChunkDirty(*neighborChunkPtr, world);
			}
		}
	}
}

void Renderer::MarkChunkDirty(Chunk& chunk, const World& world) {
	chunk.chunkMesh.dirty.store(true, std::memory_order_relaxed);
	bool expected = false;
	if (chunk.chunkMesh.queued.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
		UpdateChunkMeshAsync(chunk.shared_from_this(), world);
	}
}

void Renderer::SetControls(Controls* c) {
	controls = c;
}

void Renderer::Cleanup() {
	glDeleteBuffers(1, &constantBuffer);
}