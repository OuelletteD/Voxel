#pragma once
#include <glm.hpp>
#include "Config.h";
#include "ErrorLogger.h"

struct Voxel {
	int type;
	glm::ivec3 position;
};

struct ChunkPosition {
	int x, y;
	bool operator==(const ChunkPosition& other) const {
		return x == other.x && y == other.y;
	}
	glm::ivec3 operator*(const int multiplier) const {
		glm::ivec3 newValue = { x * multiplier, y * multiplier, 0};
		return newValue;
	}
};

namespace std {
	template <>
	struct hash<ChunkPosition> {
		size_t operator()(const ChunkPosition& pos) const {
			size_t h1 = std::hash<int>()(pos.x);
			size_t h2 = std::hash<int>()(pos.y);
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
};