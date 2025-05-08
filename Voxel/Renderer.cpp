#include "Renderer.h"
#include "string"
#include "Debugger.h"

bool Renderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
	gDevice = device;
	gContext = context;

	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(DirectX::XMMATRIX);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HRESULT hr = gDevice->CreateBuffer(&cbd, nullptr, &constantBuffer);
	if (FAILED(hr)) return false;


	//load shaders
	if (!shader.Initialize(device, L"vertexShader.hlsl", L"pixelShader.hlsl")) {
		MessageBox(nullptr, L"Shader init failed", L"Error", MB_OK);
		return false;
	}

	// Initialize cube mesh
	Vertex vertices[] = {
		// back face
		{ { -1.0f, -1.0f,  0.5f }, {1, 0, 0, 1}}, // 0 - Red
		{ {  1.0f, -1.0f,  0.5f }, {0, 1, 0, 1}}, // 1 - Green
		{ {  1.0f,  1.0f,  0.5f }, {0, 0, 1, 1}}, // 2 - Blue
		{ { -1.0f,  1.0f,  0.5f }, {1, 1, 0, 1}}, // 3 - Yellow

		{ { -1.0f, -1.0f, -0.5f }, {1, 0, 1, 1}}, // 4 - Magenta
		{ {  1.0f, -1.0f, -0.5f }, {0, 1, 1, 1}}, // 5 - Cyan
		{ {  1.0f,  1.0f, -0.5f }, {1, 0.5f, 0, 1}}, // 6 - Orange
		{ { -1.0f,  1.0f, -0.5f }, {0.5f, 0, 1, 1}}, // 7 - Purple
	};

	unsigned short indices[] = {
		// Front face (Z = 1)
		4, 5, 6,
		4, 6, 7,

		// Back face (Z = 2)
		0, 2, 1,
		0, 3, 2,

		// Left face (X = -1)
		0, 3, 7,
		0, 7, 4,

		// Right face (X = +1)
		1, 5, 6,
		1, 6, 2,

		// Top face (Y = +1)
		3, 2, 6,
		3, 6, 7,

		// Bottom face (Y = -1)
		0, 4, 5,
		0, 5, 1
	};

	if (!cubeMesh.Initialize(device, vertices, ARRAYSIZE(vertices), indices, ARRAYSIZE(indices))) return false;

	return true;
}

void Renderer::Render() {
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gContext->ClearRenderTargetView(gRenderTargetView, clearColor);

	shader.SetShaders(gContext);

	// Set the MVP matrix and update constant buffer here
	// Example: Use the camera to get matrices

	DirectX::XMMATRIX view = camera.GetViewMatrix();
	DirectX::XMMATRIX proj = camera.GetProjectionMatrix(800.0f / 600.0f);
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity(); // Default to Identity for now
	DirectX::XMMATRIX mvp = view * proj * world;

	// Update constant buffer with MVP matrix
	gContext->UpdateSubresource(constantBuffer, 0, nullptr, &mvp, 0, 0);
	gContext->VSSetConstantBuffers(0, 1, &constantBuffer); // Set the constant buffer to slot 0

	//Render the cube
	cubeMesh.Render(gContext);

	// Swap buffers, etc.
}

void Renderer::Cleanup() {
	shader.Cleanup();
	cubeMesh.Cleanup();
	if (constantBuffer) constantBuffer->Release();
}

