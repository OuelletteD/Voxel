#include "World.h"
#include <string>

const bool World::IsVoxelSolidAt(const glm::ivec3& pos) const {
	ChunkPosition chunkPosition = { pos.x / Config::CHUNK_SIZE, pos.y / Config::CHUNK_SIZE };

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