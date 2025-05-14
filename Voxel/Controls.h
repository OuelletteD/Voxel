#pragma once
#include "Camera.h"
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>
class Controls {
public:
	Controls();
	Camera camera;
	void UpdateDeltaTime();
	void ProcessKeyboard(unsigned char key);
	void ProcessMouse(int xpos, int ypos);

	glm::vec3 getCameraPosition() const;
	glm::vec3 getCameraFront() const;
	glm::vec3 getCameraUp() const;
	glm::mat4 getViewMatrix() const;
private:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float yaw;
	float pitch;
	float fov;

	float lastX;
	float lastY;
	bool firstMouse;

	float deltaTime;
	float lastFrame;

	float mouseSensitivity;
	float cameraSpeed;
	float smoothFactor;
};