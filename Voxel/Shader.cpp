#include "Shader.h"

bool Shader::CompileShader(const std::wstring& filePath, const char* entryPoint, const char* target, ID3DBlob** outBlob) {
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(filePath.c_str(), nullptr, nullptr, entryPoint, target, 0, 0, outBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}
	return true;
}

bool Shader::Initialize(ID3D11Device* device, const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath) {
	ID3DBlob* vsBlob = nullptr;
	if (!CompileShader(vertexShaderPath, "main", "vs_5_0", &vsBlob)) return false;
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(DirectX::XMFLOAT3), D3D11_INPUT_PER_VERTEX_DATA, 0 },

	};
	device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
	vsBlob->Release();

	ID3DBlob* psBlob = nullptr;
	if (!CompileShader(pixelShaderPath, "main", "ps_5_0", &psBlob)) return false;
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
	psBlob->Release();
	return true;
}

void Shader::SetShaders(ID3D11DeviceContext* context) {
	context->IASetInputLayout(inputLayout);
	context->VSSetShader(vertexShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);
}

void Shader::Cleanup() {
	if (vertexShader) vertexShader->Release();
	if (pixelShader) pixelShader->Release();
	if (inputLayout) inputLayout->Release();
}