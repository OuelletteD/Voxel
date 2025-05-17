#pragma once
#include <glm/glm.hpp>
#include <functional>

struct RaycastHit {
	glm::ivec3 blockPos;
	glm::vec3 hitPos;
	glm::vec3 normal;
	float distance;
	bool hit;
};

class VoxelRaycaster {
public:
	static RaycastHit RaycastVoxelWorld(
		glm::vec3 origin,
		glm::vec3 dir,
		float maxDistance,
		std::function<bool(glm::ivec3)> isSolidBack
	);

	//static bool IsLitBySun(glm::ivec3 voxelPos, glm::vec3 sunDir);
};