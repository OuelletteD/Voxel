#include "World.h"
#include <string>

const bool World::IsVoxelSolidAt(const glm::ivec3& pos) const {
	if (pos.z < 0 || pos.z >= Config::CHUNK_HEIGHT) return false;
	int chunkX = (pos.x >= 0) ? (pos.x / Config::CHUNK_SIZE) : ((pos.x + 1) / Config::CHUNK_SIZE - 1);
	int chunkZ = (pos.y >= 0) ? (pos.y / Config::CHUNK_SIZE) : ((pos.y + 1) / Config::CHUNK_SIZE - 1);
	ChunkPosition chunkPosition = { chunkX, chunkZ };

	auto it = chunks.find(chunkPosition);
	if (it == chunks.end()) return false;
	const Chunk& chunk = it->second;

	// Wrap voxel coordinates inside the chunk
	int localX = (pos.x % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localZ = (pos.z % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localY = pos.y;

	const Voxel* voxel = chunk.GetVoxel(localX, localY, localZ);
	return voxel && voxel->type != 0;
}

Chunk& World::CreateChunk(int chunkX, int chunkZ) {
	ChunkPosition chunkPos = { chunkX, chunkZ };
	Chunk& chunk = chunks[chunkPos];
	chunk.chunkPosition = chunkPos;
	chunk.Generate();
	return chunk;
}

void World::Generate(int xChunks, int zChunks) {
	for (int x = 0; x < xChunks; x++) {
		for (int z = 0; z < zChunks; z++) {
			Chunk chunk = CreateChunk(x, z);
		}
	}
}