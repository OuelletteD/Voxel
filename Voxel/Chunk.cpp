#include "Chunk.h"
#include <string>
#include <iostream>
void Chunk::Generate() {
	const int paddedSize = Config::CHUNK_SIZE + 2;
	int paddedHeight[paddedSize][paddedSize];
	int heightMap[Config::CHUNK_SIZE][Config::CHUNK_SIZE];
	float slopeMap[Config::CHUNK_SIZE][Config::CHUNK_SIZE];
	const int rockLine = Config::TERRAINBASEHEIGHT + Config::TERRAINHEIGHTMAX * 0.85f;
	const int baseDirtDepth = 4;
	for (int x = 0; x < paddedSize; x++) {
		for (int z = 0; z < paddedSize; z++) {
			int worldX = (x - 1) + (Config::CHUNK_SIZE * chunkPosition.x);
			int worldZ = (z - 1) + (Config::CHUNK_SIZE * chunkPosition.z);
			paddedHeight[x][z] = GetHeightAt(worldX, worldZ);
		}
	}
	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int z = 0; z < Config::CHUNK_SIZE; z++) {
			heightMap[x][z] = paddedHeight[x + 1][z + 1];
			slopeMap[x][z] = CalculateSlopeFromMap(x, z, paddedHeight);
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
				if (y == 0 || y == 1) {
					voxels[x][y][z].type = BlockType::Bedrock;
				}
				else if (y > height) {
					voxels[x][y][z].type = BlockType::Air; // air
				}
				else if (y == height) {
					if (highAltitudeRock || cliffFactor > 0.75f) {
						voxels[x][y][z].type = BlockType::Stone; // stone
					}
					else {
						voxels[x][y][z].type = BlockType::Dirt; // grass
					}
				}
				else if (y >= height - dirtDepth && cliffFactor <= 0.75f) {
					voxels[x][y][z].type = BlockType::Dirt; // dirt
				}
				else {
					voxels[x][y][z].type = BlockType::Stone; // stone
				}
				voxels[x][y][z].position = glm::ivec3(worldX, y, worldZ);
			}
		}
	}
	for (int x = 0; x < Config::CHUNK_SIZE; x++) {
		for (int z = 0; z < Config::CHUNK_SIZE; z++) {
			int terrainHeight = heightMap[x][z];
			int worldX = x + (Config::CHUNK_SIZE * chunkPosition.x);
			int worldZ = z + (Config::CHUNK_SIZE * chunkPosition.z);
			for (int y = terrainHeight + 1; y <= Config::SEALEVEL; y++) {
				//if (y < 3) continue;
				voxels[x][y][z].type = BlockType::Water; //water
			}
		}
	}
}

int Chunk::GetHeightAt(int x, int z) {
	// Macro biome layer
	float macro = CreatePerlinPoint(x, z, 0.00015f, 2); // Large-scale variation across the world
	// Params: frequency = 0.00015f (very low = broad regions), octaves = 2 (smooth)
	// Controls “biome” type or large zones of flat vs hilly terrain
	macro = pow(macro, 1.4f); // Non-linear shaping, amplifies high macro values → more hills/mountains
	macro = glm::clamp(macro, 0.1f, 1.0f);
	// Mid-scale terrain features
	float continent = CreatePerlinPoint(x, z, 0.0005f, 3); // Large continent shape (low frequency)
	float hills = CreatePerlinPoint(x, z, 0.005f, 4);      // Smaller hills (medium frequency)
	float detail = CreatePerlinPoint(x, z, 0.02f, 2);      // Fine details, tiny bumps (high frequency)
	hills *= macro; //Only apply significant hillls to high areas
	// Blend terrain layers
	float noiseValue = continent * 0.6f + hills * 0.3f + detail * 0.1f; // Weighted sum
	float flatMask = glm::smoothstep(0.25f, 0.45f, noiseValue);          // Smooth mask to flatten terrain in low-noise areas
	noiseValue -= detail * (1.0f - flatMask); // Reduce small bumps in flat regions

	// Hill shaping
	float hillShape = pow(hills, 1.8f); // Amplify hills to make them steeper
	hillShape = hillShape * 2.0f - 1.0f; // Re-center from [0,1] → [-1,1] to allow negative influence

	// Ridges / mountains
	float ridges = CreateRidgeNoise(x, z, 0.003f, 4); // Ridge pattern (low frequency)
	// Params: frequency = 0.003f (controls width of ridges), octaves = 4 (detail in ridges)
	float mountainMask = glm::smoothstep(0.7f, 0.85f, noiseValue); // Only apply ridges to higher terrain
	noiseValue += ridges * mountainMask * 0.5f; // Blend ridge noise in with scaling

	// Mountain chains
	float chain = CreatePerlinPoint(x, z, 0.0002f, 2); // Chain pattern (macro-scale)
	chain = pow(chain, 1.5f); // Exaggerates high points
	noiseValue += chain * mountainMask * 0.3f; // Adds chain variation only to high terrain

	// Shape & amplify terrain
	float shaped = noiseValue;
	shaped = glm::max(shaped, 0.0001f);        // Avoid exact 0 (prevents degenerate calculations)
	shaped = pow(shaped, 1.4f);                // Raise to a power to bias terrain heights
	noiseValue = shaped;
	noiseValue *= glm::mix(0.5f, 1.2f, macro); // Scale heights by macro biome
	// macro low → flatter; macro high → taller / rougher terrain

	// Small jitter for tiny natural variation
	float jitter = perlin.noise2D(x * 0.1f, z * 0.1f) * 0.15f; // Tiny high-frequency noise
	noiseValue += jitter / Config::TERRAINHEIGHTMAX; // Add subtle bumps
	noiseValue = glm::clamp(noiseValue, 0.0f, 1.0f); // Keep noise in [0,1]

	// Erosion-like effect
	float erosionMask = glm::smoothstep(0.18f, 0.65f, noiseValue) * (1.0f - glm::smoothstep(0.7f, 0.9f, noiseValue));
	noiseValue = glm::mix(noiseValue, glm::smoothstep(0.0f, 1.0f, noiseValue), erosionMask * 0.25f);
	// erosionMask controls where terrain is “smoothed” (lower slopes slightly flattened)

	// Apply hill steepness
	noiseValue += hillShape * 0.12f; // Steeper hills added back to terrain

	// Peaks / mountain tops 
	float peakMask = glm::smoothstep(0.75f, 0.95f, noiseValue); // Only top terrain
	noiseValue += peakMask * peakMask * 0.12f; // Extra height boost for peaks



	// Define sea level normalized to [0..1]
	const float seaLevelNorm = (float)Config::SEALEVEL/(float)Config::TERRAINHEIGHTMAX;  // same as your sea level relative to terrainheightRange

	if (noiseValue < seaLevelNorm) {
		float t = noiseValue / seaLevelNorm; // normalized [0..1] for ocean region

		// Generate ocean floor noise layers with slightly different frequencies
		float oceanNoise1 = CreatePerlinPoint(x, z, 0.0001f, 2);
		float oceanNoise2 = CreatePerlinPoint(x, z, 0.00025f, 3);

		// Blend ocean noises with different weights for variation
		float oceanNoise = oceanNoise1 * 0.7f + oceanNoise2 * 0.3f;

		// Amplify variation for ocean floor (tuned amplitude)
		float oceanVariation = glm::smoothstep(0.0f, 1.0f, oceanNoise) * 0.1f * (1.0f - t);

		// Use pow for non-linear interpolation (deeper near bottom, smoother near sea level)
		float blendFactor = pow(1.0f - t, 2.0f);

		// Final ocean floor height combines base and ocean noise
		noiseValue = std::fabs(t * seaLevelNorm + oceanVariation * seaLevelNorm * blendFactor);
	}
	int a = int(noiseValue * Config::TERRAINHEIGHTMAX + Config::TERRAINBASEHEIGHT);
	return int(noiseValue * Config::TERRAINHEIGHTMAX + Config::TERRAINBASEHEIGHT);
}

float Chunk::CalculateSlopeFromMap(int x, int z, int paddedHeight[Config::CHUNK_SIZE + 2][Config::CHUNK_SIZE + 2]) {
	int hL = paddedHeight[x + 0][z + 1];
	int hR = paddedHeight[x + 2][z + 1];
	int hD = paddedHeight[x + 1][z + 0];
	int hU = paddedHeight[x + 1][z + 2];

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