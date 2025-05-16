#include "Chunk.h"

void Chunk::Generate() {

	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int y = 0; y < Config::CHUNK_SIZE; y++) {
			for (int z = 0; z < Config::CHUNK_SIZE; z++) {
				if (y == 4) {
					voxels[x][y][z].type = 2;  // Grass
				}
				else if (y > 1 && y < 4) {
					voxels[x][y][z].type = 1;  // Dirt
				}
				else if (y == 1) {
					voxels[x][y][z].type = 3;  // stone
				}
				else {
					voxels[x][y][z].type = 0;  // Air
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