#pragma once
#include <glm/glm.hpp>
#include "Controls.h"

class Player{
public:
	Player() : position(glm::vec3(0.0f, 24.0f, 0.0f)), velocity(0.0f), colliderSize(0.6, 1.8f, 0.6f) {}
	void UpdatePlayerMovement(float deltaTime, const MovementInput& input, const glm::vec3& camFront, const glm::vec3& camRight);
	
	const glm::vec3 GetPosition() const;

private:
	void ApplyPhysics(double deltaTime);
	bool isGrounded;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 colliderSize;
	float movementSpeed = 5.0f;
	float jumpVelocity = 8.0f;
};