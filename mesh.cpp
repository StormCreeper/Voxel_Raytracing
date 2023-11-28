/*
    mesh.cpp
    author: Telo PHILIPPE

    Implementation of the Mesh class.
*/

#include "mesh.hpp"

#include <cmath>
#include <iostream>

void Mesh::initGPUGeometry(const std::vector<float> &vertexPositions, const std::vector<unsigned int> &triangleIndices) {
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)
    glGenVertexArrays(1, &m_vao);  // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.

    glBindVertexArray(m_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize = sizeof(float) * vertexPositions.size();  // Gather the size of the buffer from the CPU-side vector

    glGenBuffers(1, &m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertexPositions.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    // Same for an index buffer object that stores the list of indices of the
    // triangles forming the mesh
    size_t indexBufferSize = sizeof(unsigned int) * triangleIndices.size();
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, triangleIndices.data(), GL_DYNAMIC_READ);

    glBindVertexArray(0);  // deactivate the VAO for now, will be activated again when rendering

    m_numIndices = triangleIndices.size();
}

void Mesh::setGPUGeometry(GLuint posVbo, GLuint ibo, GLuint vao, size_t numIndices) {
    m_posVbo = posVbo;
    m_vao = vao;
    m_numIndices = numIndices;
}

void Mesh::render() const {
    glBindVertexArray(m_vao);                                                    // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);  // Call for rendering: stream the current GPU geometry through the current GPU program
    glBindVertexArray(0);                                                        // deactivate the VAO again 
}


std::shared_ptr<Mesh> Mesh::genPlane() {
    std::vector<float> vertexPositions{
        -1.0f, -1.0f, // 0
        -1.0f, 1.0f, // 1
        1.0f, 1.0f, // 2
        1.0f, -1.0f, // 3
    };
    std::vector<unsigned int> triangleIndices{
        0, 2, 1,
        0, 3, 2,
    };

    // Create a mesh object
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    mesh->initGPUGeometry(vertexPositions, triangleIndices);

    return mesh;
}


Mesh::~Mesh() {
    if(m_posVbo) glDeleteBuffers(1, &m_posVbo);
    if(m_ibo) glDeleteBuffers(1, &m_ibo);
    
    if(m_vao) glDeleteVertexArrays(1, &m_vao);
}