#include "Controls.h"

Controls::Controls()
    : cameraPos(1.0f, 2.0f, -10.0f),
    cameraFront(0.0f, 0.0f, -1.0f),
    cameraUp(0.0f, 1.0f, 0.0f),
    yaw(-90.0f),
    pitch(0.0f),
    fov(45.0f),
    lastX(400),
    lastY(300),
    firstMouse(true),
    deltaTime(0.0f),
    lastFrame(0.0f),
    mouseSensitivity(0.002f),
    cameraSpeed(10.0f),
    smoothFactor(0.9f)

{}

void Controls::UpdateDeltaTime() {
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Controls::ProcessKeyboard(unsigned char key) {
    float velocity = cameraSpeed * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
    if (key == 'w') cameraPos += cameraFront * velocity;
    if (key == 's') cameraPos -= cameraFront * velocity;
    if (key == 'a') cameraPos -= right * velocity;
    if (key == 'd') cameraPos += right * velocity;
}

void Controls::ProcessMouse(int xpos, int ypos) {
	if (firstMouse) {
		camera.lastX = xpos;
		camera.lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - camera.lastX;
    float yoffset = lastY - ypos; ////POTENTIAL ISSUE
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw = smoothFactor * yaw + (1.0f - smoothFactor) * (yaw + xoffset);
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

glm::mat4 Controls::getViewMatrix() const { return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); }
glm::vec3 Controls::getCameraPosition() const { return cameraPos; }
glm::vec3 Controls::getCameraFront() const { return cameraFront; }
glm::vec3 Controls::getCameraUp() const { return cameraUp; }