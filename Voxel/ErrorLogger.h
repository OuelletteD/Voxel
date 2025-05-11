#pragma once
#include "StringConverter.h"
#include <Windows.h>

class ErrorLogger {
public:
	static void Log(std::string message);
	static void LogOpenGLError();
};