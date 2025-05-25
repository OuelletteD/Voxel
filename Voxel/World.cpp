#include "World.h"
#include "ErrorLogger.h"

const std::optional<glm::vec3> World::GetSpawn(int x, int z) const {
	if (x > 16 || x < 0) return std::nullopt;
	if (z > 16 || z < 0) return std::nullopt;
	auto it = chunks.find(ChunkPosition{ 0, 0 });
	if (it == chunks.end()) {
		return std::nullopt;
	}
	const Chunk* chunk = it->second.get();
	for (int localY = Config::CHUNK_HEIGHT - 1; localY >= 0; --localY) {
		const Voxel* voxel = chunk->GetVoxel(x, localY, z);
		if (voxel && voxel->type != 0) {
			return (glm::vec3(x,localY,z) + glm::vec3(0.5,1,0.5));
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

glm::ivec3 World::ConvertPositionToPositionInsideChunk(glm::ivec3 position) const {
	int localX = (position.x % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localZ = (position.z % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	return glm::ivec3(localX, position.y, localZ);
}

void World::Generate(int squaredChunks) {
	std::vector<std::future<void>> futures;
	int floorChunks = floor(squaredChunks / 2.0);
	for (int x = -floorChunks; x < floorChunks; x++) {
		for (int z = -floorChunks; z < floorChunks; z++) {
			CreateChunk(ChunkPosition{ x,z });
		}
	}
	generated = true;
}
std::future<void> World::CreateChunkAsync(ChunkPosition pos) {
	{
		std::unique_lock lock(chunkMutex);
		if (chunks.contains(pos)) return std::future<void>{};
	}
	return threadPool.enqueue([this, pos]() {
		try {
		
			std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
			chunk->chunkPosition = pos;
			chunk->Generate();
			{
				std::lock_guard<std::mutex> lock(chunkBatchMutex);
				chunksPendingBatch.emplace(pos, ChunkBatchEntry(std::move(chunk)));

			}
		}
		catch (...) {
			std::lock_guard<std::mutex> lock(chunksGeneratedMutex);
			chunksBeingGenerated.erase(pos);
			throw;
		}
		{
			std::lock_guard<std::mutex> lock(chunksGeneratedMutex);
			chunksBeingGenerated.erase(pos);
		}
	});
}

void World::CreateChunk(ChunkPosition pos) {
	std::unique_lock lock(chunkMutex);
	if (chunks.contains(pos)) return;
	std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
	chunk->chunkPosition = pos;
	chunk->Generate();
	chunks[pos] = std::move(chunk);
}

void World::ProcessChunkLoadQueue(ChunkPosition playerChunk, int maxChunksPerFrame) {
	int chunksProcessed = 0;
	if (playerChunk != lastPlayerChunk) {
		SortLoadQueueByDistance(playerChunk);
		lastPlayerChunk = playerChunk;
	}
	
	while (chunksProcessed < maxChunksPerFrame) {
		ChunkPosition pos;
		{
			std::lock_guard<std::mutex> lock(priorityLoadQueueMutex);
			if (prioritizedLoadQueue.empty()) break;
			pos = prioritizedLoadQueue.front();
			prioritizedLoadQueue.erase(prioritizedLoadQueue.begin());
		}
		CreateChunkAsync(pos);
		chunksProcessed++;
	}
}

void World::SortLoadQueueByDistance(ChunkPosition playerChunk) {
	{
		std::lock_guard<std::mutex> lock(priorityLoadQueueMutex);
		if (prioritizedLoadQueue.empty()) return;
		std::sort(prioritizedLoadQueue.begin(), prioritizedLoadQueue.end(), [&](const ChunkPosition& a, const ChunkPosition& b) {
			float distA = glm::distance(glm::vec2(a.x, a.z), glm::vec2(playerChunk.x, playerChunk.z));
			float distB = glm::distance(glm::vec2(b.x, b.z), glm::vec2(playerChunk.x, playerChunk.z));
			return distA < distB;
		});
	}

}

void World::FinalizeChunkBatch() {
	constexpr auto MAX_WAIT_TIME = std::chrono::seconds(5);
	std::vector<ChunkPosition> promotableChunks;
	std::vector<ChunkPosition> forcedPromotions;
	std::unordered_set<ChunkPosition> forcedPromotionsToRemove;
	{
		std::lock_guard<std::mutex> lock(chunkBatchMutex);
		auto now = std::chrono::steady_clock::now();
		for (auto& [pos, entry] : chunksPendingBatch) {
			//promotableChunks.push_back(pos);
			if (AllNeighborsReady(pos)) {
				std::cout << "Promoting chunk" << std::endl;
				promotableChunks.push_back(pos);
			} else {
				auto elapsed = now - entry.timeAdded;
				if (elapsed > MAX_WAIT_TIME) {
					forcedPromotions.push_back(pos);
				}
			}
		}
	}
	if (promotableChunks.empty() && forcedPromotions.empty()) return;

	std::unique_lock worldLock(chunkBatchMutex);
	{
		std::unique_lock worldLock(chunkMutex);
		for (ChunkPosition pos : promotableChunks) {
			chunks[pos] = std::move(chunksPendingBatch[pos].chunk);
			chunksPendingBatch.erase(pos);
		}

		for (ChunkPosition& pos : forcedPromotions) {
			bool shouldRemove = false;
			for (const ChunkPosition& neighborPos : GetNeighbors(pos)) {
				auto promotableNeighbor = std::find(promotableChunks.begin(), promotableChunks.end(), neighborPos);
				auto forcedPromotableNeighbor = std::find(forcedPromotions.begin(), forcedPromotions.end(), neighborPos);
				if (promotableNeighbor != promotableChunks.end()) {
					shouldRemove = true;
					break;
				}

				if (forcedPromotableNeighbor != forcedPromotions.end()) {
					if (forcedPromotionsToRemove.count(neighborPos) == 0) {
						shouldRemove = true;
						break;
					}
				}
			}

			if (shouldRemove) {
				forcedPromotionsToRemove.insert(pos);
			} else {
				chunks[pos] = std::move(chunksPendingBatch[pos].chunk);
				chunksPendingBatch.erase(pos);
				promotableChunks.push_back(pos);
			}
		}
	}
	for (const ChunkPosition& pos : forcedPromotionsToRemove) {
		chunksPendingBatch.erase(pos);
	}

}

bool World::AllNeighborsReady(const ChunkPosition& position) {
	for (const ChunkPosition& neighborPos : GetNeighbors(position)) {
		if (!chunks.contains(neighborPos) && !chunksPendingBatch.contains(neighborPos))
			return false;
	}
	return true;
}

std::vector<ChunkPosition> World::GetNeighbors(const ChunkPosition& position) const {
	std::vector<ChunkPosition> neighbors;
	for (int x = -1; x <= 1; x++) {
		for (int z = -1; z <= 1; z++) {
			if (x == 0 && z == 0) continue; 
			neighbors.push_back(position + ChunkPosition{ x, z });
		}
	}
	return neighbors;
}