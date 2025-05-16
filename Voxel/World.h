#pragma once
#include <unordered_map>
#include <glm.hpp>
#include "Config.h"
#include "Chunk.h"

class Renderer;

class World {
public:
	Chunk& CreateChunk(int chunkX, int chunkY);
	void Generate(int xChunks, int yChunks);
	const bool IsVoxelSolidAt(const glm::ivec3& pos) const;
	std::unordered_map<ChunkPosition, Chunk> chunks;
};



