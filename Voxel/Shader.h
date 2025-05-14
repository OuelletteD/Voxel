#pragma once
#include <GL/glew.h>
#include <string>
#include <unordered_map>

class Shader {
public:
	GLuint program;  // OpenGL program handle
	GLuint vertexShader;  // OpenGL vertex shader handle
	GLuint fragmentShader;  // OpenGL fragment shader handle

	Shader() : program(0), vertexShader(0), fragmentShader(0) {}
	~Shader() { Cleanup(); }

	bool Initialize(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	void Use();  // Use the shader program
	void Cleanup();  // Cleanup the shaders and program
	GLuint GetProgram() const;  // Add the GetProgram function
	GLint GetUniformLocation(const std::string& name);
private:
	bool CompileShader(const std::string& filePath, GLenum shaderType, GLuint& shader);
	bool LinkProgram();
	std::unordered_map<std::string, GLint> uniformLocations;
};