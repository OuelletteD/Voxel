#pragma once
#include <unordered_map>
#include <glm.hpp>
#define CHUNK_SIZE 16

class Renderer;

struct Voxel {
	int type;
	glm::vec3 position;
};


namespace std {
	template <>
	struct hash<glm::vec3> {
		size_t operator()(const glm::vec3& pos) const {
			size_t h1 = std::hash<int>()(pos.x);
			size_t h2 = std::hash<int>()(pos.y);
			size_t h3 = std::hash<int>()(pos.z);
			return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
		}
	};
}

struct ChunkPosition {
	int x, y;

	bool operator==(const ChunkPosition& other) const {
		return x == other.x && y == other.y;
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

struct Chunk {
	Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	ChunkPosition chunkPosition;
};
class World {
public:
	Chunk CreateChunk(int chunkX, int chunkY);
	void RenderChunk(Renderer& renderer, Chunk& chunk);
private:
	std::unordered_map<ChunkPosition, Chunk> chunks;
};



