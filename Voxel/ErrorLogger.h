#pragma once
#include <iostream>

class ErrorLogger {
public:
	static void Log(std::string message);
	static void LogError(std::string message);
	static void LogOpenGLError();
};