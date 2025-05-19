#pragma once
#include "glm/glm.hpp"
#include "VoxelRaycaster.h"
#include "World.h"

struct AOOffsets {
	glm::ivec3 side1;
	glm::ivec3 side2;
	glm::ivec3 corner;
};

extern glm::ivec3 faceDirections[6];

extern glm::vec3 faceOffsets[6][4];

extern AOOffsets aoTable[6][4];

float calculateAOFactor(int faceIndex, int cornerIndex, glm::ivec3 position, std::function<bool(glm::ivec3)> isSolidAt);
std::unordered_map<glm::ivec3, bool, ivec3_hash> CalculateLighting(const World& world, Chunk& chunk, std::function<bool(glm::ivec3)> isSolidAt);