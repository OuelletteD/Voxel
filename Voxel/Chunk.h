#pragma once
#include <glm.hpp>
#include "Config.h"
#include "ErrorLogger.h"
#include "PerlinNoise.hpp"

struct Voxel {
	int type;
	glm::ivec3 position;
};

struct ChunkPosition {
	int x, z;
	bool operator==(const ChunkPosition& other) const {
		return x == other.x && z == other.z;
	}
	glm::ivec3 operator*(const int multiplier) const {
		glm::ivec3 newValue = { x * multiplier, 0, z * multiplier};
		return newValue;
	}
};

namespace std {
	template <>
	struct hash<ChunkPosition> {
		size_t operator()(const ChunkPosition& pos) const {
			size_t h1 = std::hash<int>()(pos.x);
			size_t h2 = std::hash<int>()(pos.z);
			return h1 ^ (h2 << 1);
		}
	};
}

class Chunk {
public:
	Voxel voxels[Config::CHUNK_SIZE][Config::CHUNK_SIZE][Config::CHUNK_SIZE];
	ChunkPosition chunkPosition;

	void Generate();
	const Voxel* GetVoxel(int x, int y, int z) const;
private:
	float CreatePerlinPoint(int x, int z);
	const siv::PerlinNoise::seed_type seed = 12345;
	const siv::PerlinNoise perlin{ seed };
	const float amplitude = 0.03;
	const float maxHeight = Config::CHUNK_HEIGHT;
	const int octaves = 4;
};