#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool Shader::Initialize(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
	// Compile vertex and fragment shaders
	if (!CompileShader(vertexShaderPath, GL_VERTEX_SHADER, vertexShader)) {
		std::cerr << "Failed to compile vertex shader!" << std::endl;
		return false;
	}

	if (!CompileShader(fragmentShaderPath, GL_FRAGMENT_SHADER, fragmentShader)) {
		std::cerr << "Failed to compile fragment shader!" << std::endl;
		return false;
	}

	// Link the shaders into a program
	if (!LinkProgram()) {
		std::cerr << "Failed to link shader program!" << std::endl;
		return false;
	}

	return true;
}

void Shader::Use() {
	glUseProgram(program);  // Activate the shader program
}

bool Shader::CompileShader(const std::string& filePath, GLenum shaderType, GLuint& shader) {
    // Read shader file into string
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return false;
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    std::string shaderSource = shaderStream.str();
    if (shaderSource.empty()) {
        std::cerr << "Shader source is empty!" << std::endl;
        return false;
    }
    // Create shader object
    shader = glCreateShader(shaderType);
    if (shader == 0) {
        std::cerr << "Error creating shader!" << std::endl;
        return false;
    }
    const char* source = shaderSource.c_str();
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = new char[logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        delete[] infoLog;
        return false;
    }

    return true;
}


bool Shader::LinkProgram() {
    // Create shader program
    program = glCreateProgram();

    // Attach vertex and fragment shaders
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Link the program
    glLinkProgram(program);

    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        char* infoLog = new char[logLength];
        glGetProgramInfoLog(program, logLength, &logLength, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        delete[] infoLog;
        return false;
    }

    return true;
}

GLuint Shader::GetProgram() const {
    return program;  // Return the compiled shader program
}

void Shader::Cleanup() {
	if (program) {
        glUseProgram(0);  // Unbind
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteProgram(program);
        program = 0;
	}
	if (vertexShader) {
		glDeleteShader(vertexShader);
        vertexShader = 0;
	}
	if (fragmentShader) {
		glDeleteShader(fragmentShader);
        fragmentShader = 0;
	}
}