#include "Controls.h"
#include <string>

void Controls::UpdateDeltaTime() {
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Controls::ProcessKeyboard(GLFWwindow* window) {
    float velocity = camera.cameraSpeed * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.up));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.position += camera.front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.position -= camera.front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.position -= right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.position += right * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.position += camera.up * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.position -= camera.up * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        ErrorLogger::Log(std::to_string(camera.front.x));
        ErrorLogger::Log(std::to_string(camera.front.y));
        ErrorLogger::Log(std::to_string(camera.front.z));
    }
}

void Controls::ProcessMouse(double xpos, double ypos) {
    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;
    
    lastX = xpos;
    lastY = ypos;

    ProcessMouseMovement(xoffset, yoffset);
}

void Controls::ProcessMouseMovement(double xoffset, double yoffset) {

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    camera.yaw += xoffset;
    camera.pitch -= yoffset;

    float yaw = camera.yaw;
    float pitch = camera.pitch;

    if (pitch > 89.0f) {
        camera.pitch = 89.0f;
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        camera.pitch = -89.0f;
        pitch = -89.0f;
    }

    UpdateCameraVectors(pitch, yaw);
}

void Controls::UpdateCameraVectors(float pitch, float yaw) {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera.front = glm::normalize(front);
}

void Controls::SetInitialMousePosition(float xpos, float ypos) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;  // disables first-frame delta
}