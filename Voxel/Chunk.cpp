#include "Chunk.h"
#include <string>
#include <iostream>
void Chunk::Generate() {

	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int z = 0; z < Config::CHUNK_SIZE; z++) {
			int worldX = x + (Config::CHUNK_SIZE * chunkPosition.x);
			int worldZ = z + (Config::CHUNK_SIZE * chunkPosition.z);
			float height = GetHeightAt(worldX, worldZ);
			float slope = CalculateSlope(worldX, worldZ);
			float slopeThreshold = 4.0f;

			for (int y = 0; y < Config::CHUNK_HEIGHT; y++) {
				if (y > height) {
					voxels[x][y][z].type = 0;  // Air
				}
				else if (y <= 10 || (y <= height && y >= 30)) {
					voxels[x][y][z].type = 3;  // Stone
				}
				else if (y > 10 && y <= 60) {
					if (slope > slopeThreshold) {
						voxels[x][y][z].type = 3; // stone on steep slopes (cliffs)
					}
					else {
						voxels[x][y][z].type = 1; // dirt on gentle slopes
					}
				}

				voxels[x][y][z].position = glm::ivec3(worldX, y, worldZ);
			}
		}
	}
}

int Chunk::GetHeightAt(int x, int z) {
	const int terrainheightRange = 60;
	const int terrainBaseHeight = 10;
	float noiseValue = CreatePerlinPoint(x, z);
	return round(noiseValue * terrainheightRange + terrainBaseHeight);
}

float Chunk::CalculateSlope(int x, int z) {
	int hL = GetHeightAt(x - 1, z);
	int hR = GetHeightAt(x + 1, z);
	int hD = GetHeightAt(x, z - 1);
	int hU = GetHeightAt(x, z + 1);

	float dx = float(hR - hL) / 2.0f;
	float dz = float(hU - hD) / 2.0f;

	return sqrt(dx * dx + dz * dz);
}

const Voxel* Chunk::GetVoxel(int x, int y, int z) const {
	if (x < 0 || y < 0 || z < 0 || x >= Config::CHUNK_SIZE || y >= Config::CHUNK_HEIGHT || z >= Config::CHUNK_SIZE)
		return nullptr;
	return &voxels[x][y][z];
}

float Chunk::CreatePerlinPoint(int x, int z) {
	float ridgeAmp = 0.12f;

	// Base smooth noise
	float baseNoise = perlin.octave2D_01(x * amplitude, z * amplitude, octaves);

	// Ridged noise for cliffs
	float ridgeNoise = 1.0f - fabs(perlin.octave2D_01(x * ridgeAmp, z * ridgeAmp, octaves));
	ridgeNoise = pow(ridgeNoise, 5); // Sharpen ridges (optional but effective)

	// Combine noises
	float combined = baseNoise + ridgeNoise * ridgeAmp;

	// Clamp to [0,1]
	combined = glm::clamp(combined, 0.0f, 1.0f);
	return combined;
}