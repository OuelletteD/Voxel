#include "Camera.h"
#include "Config.h"

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    float aspectRatio = float(Config::SCREEN_WIDTH) / float(Config::SCREEN_HEIGHT);
    return glm::perspective(glm::radians(Config::FOV), aspectRatio, nearPlane, farPlane);
}

void Camera::UpdateFromPlayer(const Player& player, MouseDelta mouseDelta) {
    UpdateCameraVectors(mouseDelta);
    UpdateCameraPosition(player.GetPosition());
}

void Camera::UpdateCameraPosition(const glm::vec3 playerPosition) {
    float eyeHeight = 1.62f;
    position = playerPosition + glm::vec3(0.0f, eyeHeight, 0.0f);
}

void Camera::UpdateCameraVectors(MouseDelta mouseDelta) {
    double _yaw = mouseDelta.yaw;
    double _pitch = mouseDelta.pitch;

    if (_pitch > 89.0f) {
        _pitch = 89.0f;
    }
    if (_pitch < -89.0f) {
        _pitch = -89.0f;
    }

    yaw = _yaw;
    pitch = _pitch;

    glm::vec3 newFront;
    newFront.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    newFront.y = sin(glm::radians(_pitch));
    newFront.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    front = glm::normalize(newFront);
}


std::array<Plane, 6> Camera::ExtractFrustumPlanes() {
    glm::mat4 vp = GetProjectionMatrix() * GetViewMatrix();
    std::array<Plane, 6> planes;

    // Left
    planes[0].normal.x = vp[0][3] + vp[0][0];
    planes[0].normal.y = vp[1][3] + vp[1][0];
    planes[0].normal.z = vp[2][3] + vp[2][0];
    planes[0].d = vp[3][3] + vp[3][0];

    // Right
    planes[1].normal.x = vp[0][3] - vp[0][0];
    planes[1].normal.y = vp[1][3] - vp[1][0];
    planes[1].normal.z = vp[2][3] - vp[2][0];
    planes[1].d = vp[3][3] - vp[3][0];

    // Bottom
    planes[2].normal.x = vp[0][3] + vp[0][1];
    planes[2].normal.y = vp[1][3] + vp[1][1];
    planes[2].normal.z = vp[2][3] + vp[2][1];
    planes[2].d = vp[3][3] + vp[3][1];

    // Top
    planes[3].normal.x = vp[0][3] - vp[0][1];
    planes[3].normal.y = vp[1][3] - vp[1][1];
    planes[3].normal.z = vp[2][3] - vp[2][1];
    planes[3].d = vp[3][3] - vp[3][1];

    // Near
    planes[4].normal.x = vp[0][3] + vp[0][2];
    planes[4].normal.y = vp[1][3] + vp[1][2];
    planes[4].normal.z = vp[2][3] + vp[2][2];
    planes[4].d = vp[3][3] + vp[3][2];

    // Far
    planes[5].normal.x = vp[0][3] - vp[0][2];
    planes[5].normal.y = vp[1][3] - vp[1][2];
    planes[5].normal.z = vp[2][3] - vp[2][2];
    planes[5].d = vp[3][3] - vp[3][2];

    for (Plane& p : planes) {
        float len = glm::length(p.normal);
        p.normal /= len;
        p.d /= len;
    }
    return planes;
}