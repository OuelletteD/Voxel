#pragma once
#include "Chunk.h"
#include <glm/glm.hpp>
#include "Config.h"
#include <unordered_map>

class Renderer;

class World {
public:
	World() = default;
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	World(World&&) noexcept = default;
	World& operator=(World&&) noexcept = default;
	void CreateChunk(int chunkX, int chunkZ);
	void Generate(int xChunks, int zChunks);
	const bool IsVoxelSolidAt(const glm::ivec3& pos) const;
	std::unordered_map<ChunkPosition, std::unique_ptr<Chunk>> chunks;
	bool rendered;
};



