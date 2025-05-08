#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Camera {
public:
	Camera() : position(1, 2, -10), lookAt(0, 0, 0), up(0, 1, 0) {} //more recent ? mark

	XMMATRIX GetViewMatrix();

	XMMATRIX GetProjectionMatrix(float aspectRatio);
	void Update();

private:
	XMVECTOR position;
	XMVECTOR lookAt;
	XMVECTOR up;

	const XMVECTOR DEFAULT_FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const XMVECTOR DEFAULT_UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
};