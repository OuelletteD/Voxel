#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct MovementInput {
	float forward = 0.0f; // -1 to 1
	float right = 0.0f;   // -1 to 1
	bool jump = false;
};

struct MouseDelta {
	float pitch = 0.0f;
	float yaw = 0.0f;
};

class Controls {
public:
	void ProcessKeyboard(GLFWwindow* window, double deltaTime);
	void ProcessMouse(double xpos, double ypos);
	void ProcessMouseMovement(double xoffset, double yoffset);
	MouseDelta GetMouseDelta();
	void SetInitialMousePosition(float xpos, float ypos);
	MovementInput GetMovementInput();

private:
	MovementInput input;
	MouseDelta mouseDelta;
	float lastX;
	float lastY;
	bool firstMouse;

	float mouseSensitivity = 0.03;
	bool cursorLocked = false;

};