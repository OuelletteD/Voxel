#include "World.h"
#include "Renderer.h"

Chunk World::CreateChunk(int chunkX, int chunkY) {
	Chunk chunk;

	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int y = 0; y < Config::CHUNK_SIZE; y++) {
			for (int z = 0; z < Config::CHUNK_SIZE; z++) {
				if (y == 4) {
					chunk.voxels[x][y][z].type = 2;  // Grass
				} else if (y > 1 && y < 4) {
					chunk.voxels[x][y][z].type = 1;  // Dirt
				} else if (y == 1) {
					chunk.voxels[x][y][z].type = 3;  // stone
				} else {
					chunk.voxels[x][y][z].type = 0;  // Air
				}
			}
		}
	}
	ChunkPosition chunkPos = { chunkX, chunkY };
	chunk.chunkPosition = chunkPos;
	chunks[chunkPos] = chunk;
	
	return chunk;
}

void World::RenderChunk(Renderer& renderer, Chunk& chunk) {
	for (int x = 0; x < Config::CHUNK_SIZE; ++x) {
		for (int y = 0; y < Config::CHUNK_SIZE; ++y) {
			for (int z = 0; z < Config::CHUNK_SIZE; ++z) {
				if (chunk.voxels[x][y][z].type != 0) {  // If not air
					glm::vec3 voxelPos = { x + chunk.chunkPosition.x * Config::CHUNK_SIZE, y + chunk.chunkPosition.y * Config::CHUNK_SIZE, z };
					chunk.voxels[x][y][z].position = voxelPos;
					renderer.RenderVoxel(chunk.voxels[x][y][z]);
				}
			}
		}
	}
}