#include "Mesh.h"

bool Mesh::Initialize(const Vertex* vertices, unsigned int vertexCount, const unsigned int* indices, unsigned int indexCount) {
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, indices, GL_STATIC_DRAW);  // Load data into the buffer

    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);  // Bind the vertex buffer to configure attributes

    // Position attribute: 3 floats starting at offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // TexCoord attribute: 2 floats starting at offset after position
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    // Unbind buffers and VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void Mesh::Render() {
    // Bind the VAO (this will automatically bind the VBO and EBO)
    glBindVertexArray(vao);
    // Draw the object
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    // Unbind the VAO (optional)
    glBindVertexArray(0);
}

void Mesh::SetData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    localVertices = vertices;
    localIndices = indices;
    indexCount = static_cast<unsigned int>(indices.size());
}

void Mesh::Upload() {
    if (vao == 0 || vertexBuffer == 0 || indexBuffer == 0) {
        // Lazily initialize if not already done
        Initialize(localVertices.data(), (unsigned int)localVertices.size(), localIndices.data(), (unsigned int)localIndices.size());
        return;
    }
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * localVertices.size(), localVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * localIndices.size(), localIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::Cleanup() {
    // Delete the buffers
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vertexBuffer) glDeleteBuffers(1, &vertexBuffer);
    if (indexBuffer) glDeleteBuffers(1, &indexBuffer);
}