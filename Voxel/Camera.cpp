#include "Camera.h"

XMMATRIX Camera::GetViewMatrix() {
    return XMMatrixLookAtLH(position, lookAt, up);
}

XMMATRIX Camera::GetProjectionMatrix(float aspectRatio) {
    return XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 10.0f);
}

void Camera::Update() {
    // Example: Update camera position or orientation (this could be user-controlled)
    // You might want to use input or game logic here to modify the position/rotation
    // DirectX::XMFLOAT3 newPos = ...;
    // position = newPos;

}