#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Debugger.h"
#include "ErrorLogger.h"

struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

class Mesh {
public:
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	unsigned int indexCount;

	bool Initialize(ID3D11Device* device, const Vertex* vertices, unsigned int vertexCount, const unsigned short* indices, unsigned int indexCount);
	void Render(ID3D11DeviceContext* context);
	void Cleanup();
};
