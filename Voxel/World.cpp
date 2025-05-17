#include "World.h"
#include <string>

const bool World::IsVoxelSolidAt(const glm::ivec3& pos) const {
	if (pos.y < 0 || pos.y >= Config::CHUNK_HEIGHT) return false;
	int chunkX = (pos.x >= 0) ? (pos.x / Config::CHUNK_SIZE) : ((pos.x + 1) / Config::CHUNK_SIZE - 1);
	int chunkZ = (pos.z >= 0) ? (pos.z / Config::CHUNK_SIZE) : ((pos.z + 1) / Config::CHUNK_SIZE - 1);
	ChunkPosition chunkPosition = { chunkX, chunkZ };

	auto it = chunks.find(chunkPosition);
	if (it == chunks.end()) return false;
	const Chunk& chunk = *(it->second);

	// Wrap voxel coordinates inside the chunk
	int localX = (pos.x % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localZ = (pos.z % Config::CHUNK_SIZE + Config::CHUNK_SIZE) % Config::CHUNK_SIZE;
	int localY = pos.y;

	const Voxel* voxel = chunk.GetVoxel(localX, localY, localZ);
	return voxel && voxel->type != 0;
}

void World::CreateChunk(int x, int z) {
	ChunkPosition pos{ x, z };
	auto chunk = std::make_unique<Chunk>();
	chunk->chunkPosition = pos;
	chunk->Generate();
	auto [it, inserted] = chunks.emplace(pos, std::move(chunk));
	if (!inserted) {
	}
}

void World::Generate(int xChunks, int zChunks) {
	for (int x = 0; x < xChunks; x++) {
		for (int z = 0; z < zChunks; z++) {
			CreateChunk(x, z);
		}
	}
}