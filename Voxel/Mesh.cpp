#include "Mesh.h"

bool Mesh::Initialize(const Vertex* vertices, unsigned int vertexCount, const unsigned short* indices, unsigned int indexCount) {
	this->indexCount = indexCount;

    // Generate and bind the VAO
    glGenVertexArrays(1, &vao);  // Generate the VAO
    glBindVertexArray(vao);  // Bind the VAO

	// Create Vertex Buffer (VBO)
	glGenBuffers(1, &vertexBuffer);  // Generate a new buffer object
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);  // Bind the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexCount, vertices, GL_STATIC_DRAW);  // Load data into the buffer

	// Create Index Buffer (IBO)
	glGenBuffers(1, &indexBuffer);  // Generate a new index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);  // Bind the index buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indexCount, indices, GL_STATIC_DRAW);  // Load data into the buffer

    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);  // Bind the vertex buffer to configure attributes

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));  // Position
    glEnableVertexAttribArray(0);  // Enable position attribute

    // Color attribute
    //glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));  // Color
    //glEnableVertexAttribArray(1);  // Enable color attribute

    // Unbind buffers and VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void Mesh::Render() {
    // Bind the VAO (this will automatically bind the VBO and EBO)
    glBindVertexArray(vao);
    // Draw the object
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
    // Unbind the VAO (optional)
    glBindVertexArray(0);
}

void Mesh::Cleanup() {
    // Delete the buffers
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vertexBuffer) glDeleteBuffers(1, &vertexBuffer);
    if (indexBuffer) glDeleteBuffers(1, &indexBuffer);
}