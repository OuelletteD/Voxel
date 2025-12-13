#include "Chunk.h"
#include <string>
#include <iostream>
void Chunk::Generate() {
	int heightMap[Config::CHUNK_SIZE][Config::CHUNK_SIZE];
	float slopeMap[Config::CHUNK_SIZE][Config::CHUNK_SIZE];
	const int terrainheightRange = 60;
	const int terrainBaseHeight = 10;
	const int rockLine = terrainBaseHeight + terrainheightRange * 0.85f;
	const int baseDirtDepth = 4;
	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int z = 0; z < Config::CHUNK_SIZE; z++) {
			slopeMap[x][z] = 0.0f; //> Cliff factor can't be checked at the border of a chunk.?
			int worldX = x + (Config::CHUNK_SIZE * chunkPosition.x);
			int worldZ = z + (Config::CHUNK_SIZE * chunkPosition.z);
			heightMap[x][z] = GetHeightAt(worldX, worldZ);
		}
	}
	for (int x = 1; x < Config::CHUNK_SIZE -1; x++) {
		for (int z = 1; z < Config::CHUNK_SIZE -1; z++) {
			slopeMap[x][z] = CalculateSlopeFromMap(x, z, heightMap);
		}
	}
	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int z = 0; z < Config::CHUNK_SIZE; z++) {
			int worldX = x + (Config::CHUNK_SIZE * chunkPosition.x);
			int worldZ = z + (Config::CHUNK_SIZE * chunkPosition.z);
			int height = heightMap[x][z];
			float slopeRaw = slopeMap[x][z];
			float slopeNorm = glm::clamp((slopeRaw / 2.0f), 0.0f, 1.0f);
			float cliffFactor = glm::smoothstep(0.3f, 0.7f, slopeNorm);
			int dirtDepth = int(glm::mix(baseDirtDepth, 1, cliffFactor));
			bool highAltitudeRock = height >= rockLine;
			for (int y = 0; y < Config::CHUNK_HEIGHT; y++) {
				if (y > height) {
					voxels[x][y][z].type = 0; // air
				}
				else if (y == height) {
					if (highAltitudeRock || cliffFactor > 0.75f) {
						voxels[x][y][z].type = 3; // stone
					}
					else {
						voxels[x][y][z].type = 1; // grass
					}
				}
				else if (y >= height - dirtDepth && cliffFactor <= 0.75f) {
					voxels[x][y][z].type = 1; // dirt
				}
				else {
					voxels[x][y][z].type = 3; // stone
				}

				voxels[x][y][z].position = glm::ivec3(worldX, y, worldZ);
			}
		}
	}
}

int Chunk::GetHeightAt(int x, int z) {
	const int terrainheightRange = 75;
	const int terrainBaseHeight = 10;
	//float amp = amplitude;
	float macro = CreatePerlinPoint(x, z, 0.00015f, 2);
	macro = pow(macro, 1.8f);
	
	float continent = CreatePerlinPoint(x, z, 0.0005f, 3);
	float hills = CreatePerlinPoint(x, z, 0.005f, 4);
	float detail = CreatePerlinPoint(x, z, 0.02f, 2);
	
	float noiseValue = continent * 0.6f + hills * 0.3f + detail * 0.1f;
	float flatMask = glm::smoothstep(0.25f, 0.45f, noiseValue);
	noiseValue -= detail * (1.0f - flatMask);

	float hillShape = pow(hills, 1.8f);
	hillShape = hillShape * 2.0f - 1.0f;

	float ridges = CreateRidgeNoise(x, z, 0.003f, 4);
	float mountainMask = glm::smoothstep(0.6f, 0.85f, noiseValue); //Added to terrain at higher values
	noiseValue += ridges * mountainMask * 0.5f;

	float shaped = noiseValue;
	shaped = glm::max(shaped, 0.0001f);
	shaped = pow(shaped, 1.4f);
	noiseValue = shaped;
	noiseValue *= glm::mix(0.7f, 1.15f, macro);

	float jitter = perlin.noise2D(x * 0.1f, z * 0.1f) * 0.15f;
	noiseValue += jitter / terrainheightRange;
	noiseValue = glm::clamp(noiseValue, 0.0f, 1.0f);
	float erosionMask = glm::smoothstep(0.18f, 0.65f, noiseValue) * (1.0f - glm::smoothstep(0.7f, 0.9f, noiseValue));
	noiseValue = glm::mix(noiseValue, glm::smoothstep(0.0f, 1.0f, noiseValue), erosionMask * 0.25f); //errosion
	noiseValue += hillShape * 0.12f; //Steeper hills

	float peakMask = glm::smoothstep(0.75f, 0.95f, noiseValue);
	noiseValue += peakMask * peakMask * 0.12f;
	return int(noiseValue * terrainheightRange + terrainBaseHeight);
}

float Chunk::CalculateSlopeFromMap(int x, int z, int heightMap[Config::CHUNK_SIZE][Config::CHUNK_SIZE]) {
	int hL = heightMap[x - 1][z];
	int hR = heightMap[x + 1][z];
	int hD = heightMap[x][z - 1];
	int hU = heightMap[x][z + 1];

	float dx = abs(hR - hL);
	float dz = abs(hU - hD);

	return glm::max(dx, dz);
}

const Voxel* Chunk::GetVoxel(int x, int y, int z) const {
	if (x < 0 || y < 0 || z < 0 || x >= Config::CHUNK_SIZE || y >= Config::CHUNK_HEIGHT || z >= Config::CHUNK_SIZE)
		return nullptr;
	return &voxels[x][y][z];
}

float Chunk::CreatePerlinPoint(int x, int z, float frequency, int amplitude) {
	float warpFreq = frequency * 0.4;
	float warpAmp = 8.0f;
	float warpX = perlin.octave2D_01(x * warpFreq, z * warpFreq, 2);
	float warpZ = perlin.octave2D_01((x+1000) * warpFreq, (z+1000) * warpFreq, 2);
	float nx = x + (warpX - 0.5f) * warpAmp;
	float nz = z + (warpZ - 0.5f) * warpAmp;
	//float baseNoise = perlin.octave2D_01(x * amp, z * amp, octaves);
	float baseNoise = perlin.octave2D_01(nx * frequency, nz * frequency, amplitude);
	return baseNoise;
}
float Chunk::CreateRidgeNoise(float x, float z, float frequency, int octaves) {
	float sum = 0.0f;
	float freq = frequency;
	float amp = 1.0f;
	float prev = 1.0f;

	for (int i = 0; i < octaves; i++) {

		float n = perlin.noise2D(x * freq, z * freq);
		n = 1.0f - fabs(n);
		n *= n;
		sum += n * amp * prev;
		prev = n;
		freq *= 2.0f;
		amp *= 0.5f;
	}
	return glm::clamp(sum, 0.0f, 1.0f);
}