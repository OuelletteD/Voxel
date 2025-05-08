#include "Debugger.h"

void Debugger::LogMatrix(const DirectX::XMMATRIX& matrix) {
	char str[256];
	
	// Extract the rows of the matrix
	DirectX::XMFLOAT4 row0, row1, row2, row3;
	XMStoreFloat4(&row0, matrix.r[0]);
	XMStoreFloat4(&row1, matrix.r[1]);
	XMStoreFloat4(&row2, matrix.r[2]);
	XMStoreFloat4(&row3, matrix.r[3]);

	// Log the values of the matrix
	sprintf_s(str, sizeof(str), "Matrix: \n");
	OutputDebugStringA(str);
	DebugMatrixRow(row0);
	DebugMatrixRow(row1);
	DebugMatrixRow(row2);
	DebugMatrixRow(row3);
}

void Debugger::DebugMatrixRow(const DirectX::XMFLOAT4& row) {
	std::ostringstream oss;
	oss << row.x << " " << row.y << " " << row.z << " " << row.w << "\n";
	OutputDebugStringA(oss.str().c_str());
}