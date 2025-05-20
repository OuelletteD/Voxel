#include "Player.h"
#include "Config.h"
#include <iostream>

void Player::UpdatePlayerMovement(float deltaTime, const MovementInput& input, const glm::vec3& camFront, const glm::vec3& camRight) {
	glm::vec3 moveDir = camFront * input.forward + camRight * input.right;
	float currentSpeed = input.run ? movementSpeed * 1.3 : movementSpeed;
	if (glm::length(moveDir) > 0.01f) {
		moveDir = glm::normalize(moveDir);
		velocity.x = moveDir.x * currentSpeed;
		velocity.z = moveDir.z * currentSpeed;
	}
	else {
		velocity.x = 0;
		velocity.z = 0;
	}
	if (input.jump && isGrounded) {
		velocity.y = jumpVelocity;
		isGrounded = false;
	} else if (!Config::TOGGLE_FLY && isGrounded == false) {
		velocity.y -= Config::GRAVITY * deltaTime;
	}
	ApplyPhysics(deltaTime);
	bool b = CheckCollision();
	ResolveStuck();
}

void Player::ApplyPhysics(double deltaTime) {
	float xMovement = velocity.x * static_cast<float>(deltaTime);
	float yMovement = velocity.y * static_cast<float>(deltaTime);
	float zMovement = velocity.z * static_cast<float>(deltaTime);

	position.y += yMovement;
	UpdateBoundingBox();
	if (CheckCollision()) {
		position.y -= yMovement;
		velocity.y = 0;
		isGrounded = yMovement < 0.0f;
		UpdateBoundingBox();
	} else {
		isGrounded = false;
	}
	position.x += xMovement;
	UpdateBoundingBox();
	if (CheckCollision()) {
		position.x -= xMovement;
		velocity.x = 0;
		UpdateBoundingBox();
	}
	position.z += zMovement;
	UpdateBoundingBox();
	if (CheckCollision()) {
		position.z -= zMovement;
		velocity.z = 0;
		UpdateBoundingBox();
	}
}

void Player::ResolveStuck() {
	UpdateBoundingBox();

	if (!CheckCollision()) return;

	const float maxResolveDist = 0.5f;
	const float step = 0.01f;

	for (float r = step; r <= maxResolveDist; r += step) {
		glm::vec3 directions[] = {
			{ r, 0, 0 }, {-r, 0, 0 },
			{ 0, r, 0 }, { 0,-r, 0 },
			{ 0, 0, r }, { 0, 0,-r },
		};

		for (const auto& dir : directions) {
			position += dir;
			UpdateBoundingBox();
			if (!CheckCollision()) {
				return;
			}
			position -= dir;
		}
	}
}

const glm::vec3 Player::GetPosition() const {
	return position;
}

void Player::UpdateBoundingBox() {
	glm::vec3 halfSize = glm::vec3(0.3f, 1.8f, 0.3f);
	boundingBox.min = position + glm::vec3(-halfSize.x, 0.0f, -halfSize.z);
	boundingBox.max = position + halfSize;
}

bool Player::CheckCollision() {
	int xMin = floor(boundingBox.min.x);
	int xMax = floor(boundingBox.max.x);
	int yMin = floor(boundingBox.min.y);
	int yMax = floor(boundingBox.max.y);
	int zMin = floor(boundingBox.min.z);
	int zMax = floor(boundingBox.max.z);
	for (int x = xMin; x <= xMax; ++x) {
		for (int y = yMin; y <= yMax; ++y) {
			for (int z = zMin; z <= zMax; ++z) {
				if (world.IsVoxelSolidAt(glm::ivec3(x, y, z))) {
					return true;
				}
			}
		}
	}
	return false;
}