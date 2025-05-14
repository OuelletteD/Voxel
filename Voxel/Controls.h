#pragma once
#include "Camera.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>
#include <GLFW/glfw3.h>
class Controls {
public:
	Controls(Camera& cam) : 
		camera(cam),
		lastX(400),
		lastY(300),
		firstMouse(true),
		deltaTime(0.0),
		lastFrame(0.0),
		mouseSensitivity(0.03f) {};
	void UpdateDeltaTime();
	void ProcessKeyboard(GLFWwindow* window);
	void ProcessMouse(double xpos, double ypos);
	void ProcessMouseMovement(double xoffset, double yoffset);
	void UpdateCameraVectors(float pitch, float yaw);
	void SetInitialMousePosition(float xpos, float ypos);

private:
	Camera& camera;
	float lastX;
	float lastY;
	bool firstMouse;

	double deltaTime;
	double lastFrame;

	float mouseSensitivity;
};