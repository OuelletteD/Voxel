#include "Mesh.h"

bool Mesh::Initialize(ID3D11Device* device, const Vertex* vertices, unsigned int vertexCount, const unsigned short* indices, unsigned int indexCount) {
	this->indexCount = indexCount;

	//Create Vertex Buffer
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vertex) * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;
	HRESULT hr = device->CreateBuffer(&vbd, &initData, &vertexBuffer);
	if (FAILED(hr)) return false;

	//Create Index Buffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(unsigned short) * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices;
	hr = device->CreateBuffer(&ibd, &initData, &indexBuffer);
	if (FAILED(hr)) return false;

	return true;
}

void Mesh::Render(ID3D11DeviceContext* context) {
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->DrawIndexed(indexCount, 0, 0);
}

void Mesh::Cleanup() {
	if (vertexBuffer) vertexBuffer->Release();
	if (indexBuffer) indexBuffer->Release();
}