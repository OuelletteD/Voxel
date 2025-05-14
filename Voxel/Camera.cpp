#include "Camera.h"
#include <GL/freeglut.h>

glm::mat4 Camera::GetViewMatrix() const {
    // Using glm::lookAt to create the view matrix
    return glm::lookAt(position, lookAt, up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const {
    // For perspective projection: 
    // OpenGL uses glm::perspective (left-handed coordinate system)
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);  // Near and far planes are set
}

void Camera::Update() {
    if (INPUT_MOUSE) {
        ErrorLogger::Log("Error");
    }
    // You can add movement or camera rotation logic here if needed
}