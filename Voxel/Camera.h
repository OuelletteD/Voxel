#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // For glm::lookAt and glm::perspective
#include "Player.h"
#include <array>

struct Plane {
	glm::vec3 normal;
	float d;

	float DistanceToPoint(const glm::vec3& point) const {
		return glm::dot(normal, point) + d;
	}
};

class Camera {
public:
	Camera(glm::vec3 startPos, glm::vec3 startFront, glm::vec3 startUp)
		: position(startPos), front(startFront), up(startUp), yaw(glm::degrees(atan2(startFront.z, startFront.x))), pitch(glm::degrees(asin(startFront.y))), fov(45.0f) {
	}

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;
	void UpdateFromPlayer(const Player& player, MouseDelta mouseDelta);

	void SetPosition(const glm::vec3& newPosition) { position = newPosition; }
	void SetFront(const glm::vec3& newLookAt) { front = newLookAt; }
	void SetUp(const glm::vec3& newUp) { up = newUp; }
	std::array<Plane, 6> ExtractFrustumPlanes();

	glm::vec3 GetFront() const { return front; }
	glm::vec3 GetRight() const { return glm::normalize(glm::cross(front, up)); }
	glm::vec3 GetPosition() const { return position; }

private:
	void UpdateCameraPosition(glm::vec3);
	void UpdateCameraVectors(MouseDelta mouseDelta);
	double yaw; //(left to right movement : pointing down -Z)
	double pitch; //(Up down movement)
	float fov;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;
};