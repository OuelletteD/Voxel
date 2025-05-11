#pragma once
#include <iostream>
#include <GL/glew.h>

class ErrorLogger {
public:
	static void Log(std::string message);
	static void LogError(std::string message);
	static void LogOpenGLError();
};