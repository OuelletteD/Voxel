#pragma once
#include <glm/glm.hpp>
#include "Config.h"
#include "ErrorLogger.h"
#include "PerlinNoise.hpp"
#include "Mesh.h"

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
			size_t seed = std::hash<int>()(pos.x);
			seed ^= std::hash<int>()(pos.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};
}

class Chunk {
public:
	Chunk() : perlin(Config::WOLRD_SEED) {}
	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;

	// Enable moving:
	Chunk(Chunk&&) = default;
	Chunk& operator=(Chunk&&) = default;

	Voxel voxels[Config::CHUNK_SIZE][Config::CHUNK_HEIGHT][Config::CHUNK_SIZE];
	ChunkPosition chunkPosition;
	ChunkMesh chunkMesh;

	void Generate();
	const Voxel* GetVoxel(int x, int y, int z) const;

	int exampleData = 0;
	
private:
	float CreatePerlinPoint(int x, int z);
	const siv::PerlinNoise perlin{ Config::WOLRD_SEED };
	const float amplitude = 0.0015;
	const int octaves = 6;
};