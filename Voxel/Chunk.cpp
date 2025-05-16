#include "Chunk.h"
#include <string>

void Chunk::Generate() {
	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int z = 0; z < Config::CHUNK_SIZE; z++) {
			int height = round(CreatePerlinPoint(x + (Config::CHUNK_SIZE * chunkPosition.x), z + (Config::CHUNK_SIZE * chunkPosition.z)) * maxHeight);
			for (int y = 0; y < Config::CHUNK_HEIGHT; y++) {
				
				if (y > height) {
					voxels[x][y][z].type = 0;  // Air
				}
				else if (y > 4 && y < 10) {
					voxels[x][y][z].type = 1;  // Dirt
				}
				else if (y <= 4) {
					voxels[x][y][z].type = 3;  // stone
				}
			}
		}
	}
}

const Voxel* Chunk::GetVoxel(int x, int y, int z) const {
	if (x < 0 || y < 0 || z < 0 || x >= Config::CHUNK_SIZE || y >= Config::CHUNK_SIZE || z >= Config::CHUNK_SIZE)
		return nullptr;
	return &voxels[x][y][z];
}

float Chunk::CreatePerlinPoint(int x, int z) {
	return perlin.octave2D_01((x * amplitude), (z * amplitude), octaves);
}