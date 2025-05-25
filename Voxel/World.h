#pragma once
#include "Chunk.h"
#include <glm/glm.hpp>
#include "Config.h"
#include <unordered_map>
#include <optional>
#include <future>
#include "ThreadPool.h"
#include "unordered_set"
#include "queue"
#include "MainThreadDispatcher.h"
#include <shared_mutex>

class Renderer;

struct ChunkBatchEntry {
	std::shared_ptr<Chunk> chunk;
	std::chrono::steady_clock::time_point timeAdded;

	ChunkBatchEntry(std::shared_ptr<Chunk> c)
		: chunk(std::move(c)), timeAdded(std::chrono::steady_clock::now()) {
	}
	ChunkBatchEntry() = default; 
};

class World {
public:
	World(ThreadPool& threadPool, MainThreadDispatcher& mtd) : threadPool(threadPool), mtd(mtd) {}
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	World(World&&) noexcept = default;
	World& operator=(World&&) noexcept = default;

	void CreateChunk(ChunkPosition pos);
	std::future<void> CreateChunkAsync(ChunkPosition pos);
	void Generate(int squaredChunks);
	ChunkPosition GetChunkPositionFromPlayerCoordinates(glm::vec3 position) const;
	void ProcessChunkLoadQueue(ChunkPosition playerChunk, int maxChunksPerFrame = 1);
	void FinalizeChunkBatch();
	ChunkPosition GetChunkPositionFromCoordinates(glm::vec3 position) const;
	glm::ivec3 ConvertPositionToPositionInsideChunk(glm::ivec3 position) const;
	void SortLoadQueueByDistance(ChunkPosition playerChunk);
	const std::optional<glm::vec3> GetSpawn(int x, int z) const;

	bool rendered = false;
	bool generated = false;
	
	mutable std::shared_mutex chunkMutex;
	std::mutex priorityLoadQueueMutex;
	std::mutex chunksGeneratedMutex;
	std::vector<ChunkPosition> prioritizedLoadQueue;
	std::unordered_set<ChunkPosition> chunksBeingGenerated;
	std::unordered_map<ChunkPosition, std::shared_ptr<Chunk>> chunks;
private:	
	bool AllNeighborsReady(const ChunkPosition& pos);
	std::vector<ChunkPosition> GetNeighbors(const ChunkPosition& position) const;

	ThreadPool& threadPool;
	MainThreadDispatcher& mtd;
	std::unordered_map<ChunkPosition, ChunkBatchEntry> chunksPendingBatch;
	std::mutex chunkBatchMutex;
	ChunkPosition lastPlayerChunk = { INT_MIN, INT_MIN };

};



