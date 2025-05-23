#include "World.h"
#include "ErrorLogger.h"

const bool World::IsVoxelSolidAt(const glm::ivec3& pos) const {
	if (pos.y < 0 || pos.y >= Config::CHUNK_HEIGHT) return false;

	ChunkPosition chunkPosition = GetChunkPositionFromCoordinates(pos);
	const Chunk* chunk = GetChunkAtPosition(chunkPosition);
	if (!chunk) return false;
	glm::ivec3 localPosition = ConvertPositionToPositionInsideChunk(pos);

	const Voxel* voxel = chunk->GetVoxel(localPosition.x, localPosition.y, localPosition.z);
	return voxel && voxel->type != 0;
}

const std::optional<glm::ivec3> World::GetPositionAtXZ(const int x, const int z) const {
	ChunkPosition chunkPosition = GetChunkPositionFromCoordinates(glm::ivec3(x, 0, z));
	const Chunk* chunk = GetChunkAtPosition(chunkPosition);
	if (!chunk) return std::nullopt;
	glm::ivec3 localPosition = ConvertPositionToPositionInsideChunk(glm::vec3(x, 0, z));

	for (int localY = Config::CHUNK_HEIGHT - 1; localY >= 0; --localY) {
		const Voxel* voxel = chunk->GetVoxel(localPosition.x, localY, localPosition.z);
		if (voxel && voxel->type != 0) {
			return glm::ivec3(x, localY, z); // Return world-space Y
		}
	}
	
	return std::nullopt;
}

ChunkPosition World::GetChunkPositionFromPlayerCoordinates(glm::vec3 position) const {
	int posX = floor(position.x);
	int posZ = floor(position.z);
	return GetChunkPositionFromCoordinates(glm::ivec3(posX, 0, posZ));
}

ChunkPosition World::GetChunkPositionFromCoordinates(glm::vec3 position) const {
	int chunkX = (position.x >= 0) ? (position.x / Config::CHUNK_SIZE) : ((position.x + 1) / Config::CHUNK_SIZE - 1);
	int chunkZ = (position.z >= 0) ? (position.z / Config::CHUNK_SIZE) : ((position.z + 1) / Config::CHUNK_SIZE - 1);
	return { chunkX, chunkZ };
}

const Chunk* World::GetChunkAtPosition(ChunkPosition position) const {
	std::lock_guard<std::mutex> lock(chunkMutex);
	auto it = chunks.find(position);
	if (it == chunks.end()) {
		return nullptr;
	}
	return it->second.get();
}

glm::ivec3 World::ConvertPositionToPositionInsideChunk(glm::ivec3 position) const {
	int localX = (position.x % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localZ = (position.z % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	return glm::ivec3(localX, position.y, localZ);
}

void World::Generate(int squaredChunks) {
	int floorX = -floor(squaredChunks / 2);
	int ceilX = ceil(squaredChunks / 2);
	for (int x = floorX; x < ceilX; x++) {
		for (int z = floorX; z < ceilX; z++) {
			CreateChunkAsync(ChunkPosition{ x,z });
		}
	}
}
void World::CreateChunkAsync(ChunkPosition pos) {
	{
		std::lock_guard<std::mutex> lock(chunkMutex);
		if (chunks.contains(pos) || chunksBeingGenerated.contains(pos)) return;
		chunksBeingGenerated.insert(pos);
	}
	threadPool.enqueue([this, pos]() {
		try {
		
			std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
			chunk->chunkPosition = pos;
			chunk->Generate();

			std::lock_guard<std::mutex> lock(chunkMutex);
			chunks.emplace(pos, std::move(chunk));
		}
		catch (...) {
			// On failure, remove from chunksBeingGenerated so it can be retried
			std::lock_guard<std::mutex> lock(chunkMutex);
			chunksBeingGenerated.erase(pos);
			throw;
		}
		// Finally remove pos from chunksBeingGenerated
		{
			std::lock_guard<std::mutex> lock(chunkMutex);
			chunksBeingGenerated.erase(pos);
		}
	});
}

void World::ProcessChunkLoadQueue(int maxChunksPerFrame) {
	int chunksProcessed = 0;
	while (chunksProcessed < maxChunksPerFrame) {
		ChunkPosition pos;
		{
			std::lock_guard<std::mutex> lock(chunkLoadQueueMutex);
			if (chunkLoadQueue.empty()) break;
			pos = chunkLoadQueue.front();
			chunkLoadQueue.pop();
		}
		CreateChunkAsync(pos);
		chunksProcessed++;
	}
}