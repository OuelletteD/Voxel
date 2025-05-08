#pragma once
#include <DirectXMath.h>
#include <windows.h>
#include <stdio.h>
#include <sstream>
#include "Mesh.h"

class Debugger {
public:
	static void LogMatrix(const DirectX::XMMATRIX& matrix);
private:
	static void DebugMatrixRow(const DirectX::XMFLOAT4& row);
};