#include "ErrorLogger.h"
#include <glad/glad.h>

void ErrorLogger::Log(std::string message) {
	std::cerr << message << std::endl;
}

void ErrorLogger::LogError(std::string message) {
	std::cerr << "Error: " << message << std::endl;
}

void ErrorLogger::LogOpenGLError() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << err << std::endl;
	}
}