#pragma once
#include <glm/glm.hpp>
#include "Controls.h"
#include "world.h"

struct AABB { //Axis aligned bounding box
	glm::vec3 min;
	glm::vec3 max;
};

class Player{
public:
	Player(const World& worldRef) : position(glm::vec3(1.0f, 58.0f, 1.0f)), velocity(0.0f), world(worldRef) {}
	void UpdatePlayerMovement(float deltaTime, const MovementInput& input, const glm::vec3& camFront, const glm::vec3& camRight);
	
	const glm::vec3 GetPosition() const;
	void SetPosition(glm::vec3 pos);
	ChunkPosition GetChunk();

private:
	void ApplyPhysics(double deltaTime);
	bool isGrounded;
	glm::vec3 position;
	glm::vec3 velocity;
	ChunkPosition chunkPosition;
	const World& world;
	AABB boundingBox;
	float movementSpeed = 5.0f;
	float jumpVelocity = 5.0f;
	std::shared_mutex localChunkMutex;
	bool CheckCollision();
	void UpdateBoundingBox();
	void ResolveStuck();
	void UpdateLocalChunks();
	std::array<const Chunk*, Config::CHUNK_SIZE* Config::CHUNK_SIZE> localChunkCache = { nullptr };
	bool IsVoxelSolidCached(const glm::ivec3& pos) const;
	static int ChunkToIndex(int dx, int dz) {
		return (dx + 1) + (dz + 1) * 3;
	}
};