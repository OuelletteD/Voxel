#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

struct MovementInput {
	float forward = 0.0f; // -1 to 1
	float right = 0.0f;   // -1 to 1
	bool jump = false;
	bool run = false;
};

struct MouseDelta {
	float pitch = 0.0f;
	float yaw = 0.0f;
};

class Controls {
public:
	void Initialize(GLFWwindow* window);
	void ProcessKeyboard(GLFWwindow* window, double deltaTime);
	void ProcessMouse(double xpos, double ypos);
	void ProcessMouseMovement(double xoffset, double yoffset);
	MouseDelta GetMouseDelta();
	void SetInitialMousePosition(double xpos, double ypos);
	MovementInput GetMovementInput();
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	MovementInput input;
	MouseDelta mouseDelta;
	double lastX;
	double lastY;
	bool firstMouse;

	float mouseSensitivity = 0.03;
	bool cursorLocked = false;

};