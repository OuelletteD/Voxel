#include "Controls.h"
#include "Config.h"
#include <iostream>

void Controls::ProcessKeyboard(GLFWwindow* window, double deltaTime) {
    input = {};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) input.forward += 1;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input.forward -= 1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input.right -= 1;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input.right += 1;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) input.jump = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) input.run = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) input.run = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        SetInitialMousePosition(Config::SCREEN_WIDTH / 2.0f, Config::SCREEN_HEIGHT / 2.0f);
        glfwSetInputMode(window, GLFW_CURSOR, (cursorLocked ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED));
        cursorLocked = !cursorLocked;
    }
}

void Controls::ProcessMouse(double xpos, double ypos) {
    double xoffset = xpos - lastX;
    double yoffset = ypos - lastY;
    
    lastX = xpos;
    lastY = ypos;
    ProcessMouseMovement(xoffset, yoffset);
}

void Controls::ProcessMouseMovement(double xoffset, double yoffset) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    mouseDelta.pitch -= yoffset;
    mouseDelta.yaw += xoffset;

}

MovementInput Controls::GetMovementInput() {
    return input;
}

MouseDelta Controls::GetMouseDelta() {
    return mouseDelta;
}

void Controls::SetInitialMousePosition(float xpos, float ypos) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;  // disables first-frame delta
}