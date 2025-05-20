#include "Player.h"
#include "Config.h"

void Player::UpdatePlayerMovement(float deltaTime, const MovementInput& input, const glm::vec3& camFront, const glm::vec3& camRight) {
	glm::vec3 moveDir = camFront * input.forward + camRight * input.right;
	if (glm::length(moveDir) > 0.01f) {
		moveDir = glm::normalize(moveDir);
		velocity.x = moveDir.x * movementSpeed;
		velocity.z = moveDir.z * movementSpeed;
	}
	else {
		velocity.x = 0;
		velocity.z = 0;
	}
	//Apply movement
	if (input.jump && isGrounded) {
		//velocity.y = jumpStrength;
		isGrounded = false; // You'll need to track this based on collisions or position
	}
	//handle jump
	ApplyPhysics(deltaTime);
}

void Player::ApplyPhysics(double deltaTime) {
	if (!Config::TOGGLE_FLY) {
		// Apply gravity to vertical velocity
		//velocity.y -= Config::GRAVITY * deltaTime;

		// Update position by velocity
	}
	position += velocity * static_cast<float>(deltaTime);
}

const glm::vec3 Player::GetPosition() const {
	return position;
}