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

class Renderer;

class World {
public:
	World(ThreadPool& threadPool, MainThreadDispatcher& mtd) : threadPool(threadPool), mtd(mtd) {}
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	World(World&&) noexcept = default;
	World& operator=(World&&) noexcept = default;
	void CreateChunkAsync(ChunkPosition pos);
	void Generate(int squaredChunks);
	const bool IsVoxelSolidAt(const glm::ivec3& pos) const;
	const std::optional<glm::ivec3> GetPositionAtXZ(const int x, const int z) const;
	std::unordered_map<ChunkPosition, std::shared_ptr<Chunk>> chunks;
	bool rendered = false;
	ChunkPosition GetChunkPositionFromPlayerCoordinates(glm::vec3 position) const;
	mutable std::mutex chunkMutex;
	std::mutex chunkLoadQueueMutex;
	std::queue<ChunkPosition> chunkLoadQueue;
	std::unordered_set<ChunkPosition> chunksBeingGenerated;
	void ProcessChunkLoadQueue(int maxChunksPerFrame = 1);
private:
	const Chunk* GetChunkAtPosition(ChunkPosition position) const;
	glm::ivec3 ConvertPositionToPositionInsideChunk(glm::ivec3 position) const;
	ChunkPosition GetChunkPositionFromCoordinates(glm::vec3 position) const;
	ThreadPool& threadPool;
	MainThreadDispatcher& mtd;

};



