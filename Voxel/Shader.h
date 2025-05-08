#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <DirectXMath.h>

class Shader {
public:
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;

	bool Initialize(ID3D11Device* device, const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath);
	void SetShaders(ID3D11DeviceContext* context);
	void Cleanup();
private:
	bool CompileShader(const std::wstring& filePath, const char* entryPoint, const char* target, ID3DBlob** outBlob);
};