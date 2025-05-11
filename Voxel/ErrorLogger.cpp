#include "ErrorLogger.h"
#include <iostream>
#include <GL/glew.h>
void ErrorLogger::Log(std::string message) {
	std::cerr << message << std::endl;
}

void ErrorLogger::LogOpenGLError() {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << err << std::endl;
	}
}