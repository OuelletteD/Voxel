#include "VoxelRaycaster.h"
#include <string>
#include <iostream>

RaycastHit VoxelRaycaster::RaycastVoxelWorld(glm::vec3 origin,
	glm::vec3 dir,
	float maxDistance,
	std::function<bool(glm::ivec3)> isSolidBack) {
	RaycastHit result{};
	bool firstVoxel = true;
	dir = -glm::normalize(dir);
	glm::ivec3 voxel = glm::floor(origin);
	auto safeDiv = [](float a, float b) -> float {
		return (b == 0.0f) ? std::numeric_limits<float>::max() : a / b;
	};
	glm::vec3 deltaDist = glm::abs(glm::vec3(
		safeDiv(1.0f, dir.x),
		safeDiv(1.0f, dir.y),
		safeDiv(1.0f, dir.z)
	));

	glm::ivec3 step = glm::ivec3(dir.x > 0 ? 1 : -1, dir.y > 0 ? 1 : -1, dir.z > 0 ? 1 : -1);
	glm::vec3 sideDist;
	sideDist.x = (step.x > 0 ? (voxel.x + 1.0f - origin.x) : (origin.x - voxel.x)) * deltaDist.x;
	sideDist.y = (step.y > 0 ? (voxel.y + 1.0f - origin.y) : (origin.y - voxel.y)) * deltaDist.y;
	sideDist.z = (step.z > 0 ? (voxel.z + 1.0f - origin.z) : (origin.z - voxel.z)) * deltaDist.z;
	float traveled = 0.0f;
	glm::ivec3 normal{ 0 };

	while (traveled < maxDistance) {
		if (!firstVoxel && isSolidBack(voxel)) {
			result.blockPos = voxel;
			result.hitPos = origin + dir * traveled;
			result.normal = glm::vec3(normal);
			result.distance = traveled;
			result.hit = true;
			return result;
		}
		firstVoxel = false;
		if (sideDist.x < sideDist.y) {
			if (sideDist.x < sideDist.z) {
				voxel.x += step.x;
				traveled = sideDist.x;
				sideDist.x += deltaDist.x;
				normal = { -step.x, 0, 0 };
			}
			else {
				voxel.z += step.z;
				traveled = sideDist.z;
				sideDist.z += deltaDist.z;
				normal = { 0,0,-step.z };
			}
		}
		else {
			if (sideDist.y < sideDist.z) {
				voxel.y += step.y;
				traveled = sideDist.y;
				sideDist.y += deltaDist.y;
				normal = { 0, -step.y, 0 };
			}
			else {
				voxel.z += step.z;
				traveled = sideDist.z;
				sideDist.z += deltaDist.z;
				normal = { 0, 0, -step.z };
			}
		}
	}
	return result;
}

/*
bool VoxelRaycaster::IsLitBySun(glm::ivec3 voxelPos, glm::vec3 sunDir, std::function<bool(glm::ivec3)> isSolidBack) {
	glm::vec3 center = glm::vec3(voxelPos) + glm::vec3(0.5f);
	RaycastHit hit = RaycastVoxelWorld(center, sunDir, 100.0f, [](glm::ivec3 p) {
		return isSolidBack;
	});
	return !hit.hit;
}*/