#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // For glm::lookAt and glm::perspective
#include "ErrorLogger.h"

class Camera {
public:
	Camera(glm::vec3 startPos, glm::vec3 startFront, glm::vec3 startUp)
		: position(startPos), front(startFront), up(startUp), yaw(glm::degrees(atan2(startFront.z, startFront.x))), pitch(glm::degrees(asin(startFront.y))), fov(45.0f) {
	}

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float aspectRatio) const;

	void SetPosition(const glm::vec3& newPosition) { position = newPosition; }
	void SetFront(const glm::vec3& newLookAt) { front = newLookAt; }
	void SetUp(const glm::vec3& newUp) { up = newUp; }

	float cameraSpeed = 10.0f;
	double yaw; //(left to right movement : pointing down -Z)
	double pitch; //(Up down movement)
	float fov;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;
};