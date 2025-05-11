#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>  // For glm::lookAt and glm::perspective

class Camera {
public:
	Camera() : position(1.0f, 2.0f, -10.0f), lookAt(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f) {}


	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float aspectRatio) const;
	void Update();  // For updating the camera (if needed)

	void SetPosition(const glm::vec3& newPosition) { position = newPosition; }
	void SetLookAt(const glm::vec3& newLookAt) { lookAt = newLookAt; }
	void SetUp(const glm::vec3& newUp) { up = newUp; }

private:
	glm::vec3 position;
	glm::vec3 lookAt;
	glm::vec3 up;

	const glm::vec3 DEFAULT_FORWARD_VECTOR = glm::vec3(0.0f, 0.0f, 1.0f);
	const glm::vec3 DEFAULT_UP_VECTOR = glm::vec3(0.0f, 1.0f, 0.0f);
};