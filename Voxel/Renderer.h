#pragma once
#include <d3d11.h>
#include "D3DResources.h"
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"

class Renderer {
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
	void Render();
	void Cleanup();
private:
	Shader shader;
	Mesh cubeMesh;
	Camera camera;
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gContext = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
};