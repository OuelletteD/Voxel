#include "Camera.h"

glm::mat4 Camera::GetViewMatrix() const {
    // Using glm::lookAt to create the view matrix
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const {
    // For perspective projection: 
    // OpenGL uses glm::perspective (left-handed coordinate system)
    return glm::perspective(glm::radians(45.0f), aspectRatio, nearPlane, farPlane);  // Near and far planes are set
}
