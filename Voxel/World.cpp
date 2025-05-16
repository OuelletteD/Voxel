#include "World.h"
#include <string>

const bool World::IsVoxelSolidAt(const glm::ivec3& pos) const {
	if (pos.z < 0 || pos.z >= Config::CHUNK_HEIGHT) return false;
	int chunkX = (pos.x >= 0) ? (pos.x / Config::CHUNK_SIZE) : ((pos.x + 1) / Config::CHUNK_SIZE - 1);
	int chunkY = (pos.y >= 0) ? (pos.y / Config::CHUNK_SIZE) : ((pos.y + 1) / Config::CHUNK_SIZE - 1);
	ChunkPosition chunkPosition = { chunkX, chunkY };

	auto it = chunks.find(chunkPosition);
	if (it == chunks.end()) return false;
	const Chunk& chunk = it->second;

	// Wrap voxel coordinates inside the chunk
	int localX = (pos.x % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localY = (pos.y % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localZ = pos.z;

	const Voxel* voxel = chunk.GetVoxel(localX, localY, localZ);
	return voxel && voxel->type != 0;
}

Chunk& World::CreateChunk(int chunkX, int chunkY) {
	ChunkPosition chunkPos = { chunkX, chunkY };
	Chunk& chunk = chunks[chunkPos];
	chunk.chunkPosition = chunkPos;
	chunk.Generate();
	return chunk;
}

void World::Generate(int xChunks, int yChunks) {
	for (int x = 0; x < xChunks; x++) {
		for (int y = 0; y < xChunks; y++) {
			Chunk chunk = CreateChunk(x, y);
		}
	}
}