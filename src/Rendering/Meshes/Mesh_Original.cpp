#include "Mesh.h"
#include "../../Core/Logging/Logger.h"
#include "../Core/OpenGLHeaders.h"
#include <cmath>

namespace GameEngine {

Mesh::Mesh() = default;

Mesh::~Mesh() {
    CleanupBuffers();
}

void Mesh::SetVertices(const std::vector<Vertex>& vertices) {
    m_vertices = vertices;
    m_uploaded = false;
}

void Mesh::SetIndices(const std::vector<unsigned int>& indices) {
    m_indices = indices;
    m_uploaded = false;
}

void Mesh::Upload() {
    if (m_vertices.empty()) {
        Logger::Warning("Attempting to upload mesh with no vertices");
        return;
    }
    
    CleanupBuffers();
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    if (!m_indices.empty()) {
        glGenBuffers(1, &m_EBO);
    }
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
    
    if (!m_indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
    }
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    m_uploaded = true;
    Logger::Debug("Mesh uploaded with " + std::to_string(m_vertices.size()) + " vertices and " + std::to_string(m_indices.size()) + " indices");
}

void Mesh::Bind() const {
    if (m_VAO != 0) {
        glBindVertexArray(m_VAO);
    }
}

void Mesh::Unbind() const {
    glBindVertexArray(0);
}

void Mesh::Draw() const {
    if (!m_uploaded || m_VAO == 0) {
        Logger::Warning("Attempting to draw mesh that hasn't been uploaded");
        return;
    }
    
    if (!m_indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    }
}

void Mesh::CleanupBuffers() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    
    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    
    m_uploaded = false;
}

Mesh Mesh::CreateCube(float size) {
    Mesh mesh;
    
    float halfSize = size * 0.5f;
    
    std::vector<Vertex> vertices = {
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(0, 0, 1), Vector3(0, 0, 0)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(0, 0, 1), Vector3(1, 0, 0)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(0, 0, 1), Vector3(1, 1, 0)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(0, 0, 1), Vector3(0, 1, 0)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(0, 0, -1), Vector3(1, 0, 0)),
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(0, 0, -1), Vector3(1, 1, 0)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(0, 0, -1), Vector3(0, 1, 0)),
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(0, 0, -1), Vector3(0, 0, 0)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(-1, 0, 0), Vector3(0, 0, 0)),
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(-1, 0, 0), Vector3(1, 0, 0)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(-1, 0, 0), Vector3(1, 1, 0)),
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(-1, 0, 0), Vector3(0, 1, 0)),
        
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(1, 0, 0), Vector3(1, 0, 0)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(1, 0, 0), Vector3(1, 1, 0)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(1, 0, 0), Vector3(0, 1, 0)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(1, 0, 0), Vector3(0, 0, 0)),
        
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(0, 1, 0), Vector3(0, 1, 0)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(0, 1, 0), Vector3(0, 0, 0)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(0, 1, 0), Vector3(1, 0, 0)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(0, 1, 0), Vector3(1, 1, 0)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(0, -1, 0), Vector3(1, 1, 0)),
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(0, -1, 0), Vector3(0, 1, 0)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(0, -1, 0), Vector3(0, 0, 0)),
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(0, -1, 0), Vector3(1, 0, 0))
    };
    
    std::vector<unsigned int> indices = {
        0,  1,  2,   2,  3,  0,   // Front
        4,  5,  6,   6,  7,  4,   // Back
        8,  9,  10,  10, 11, 8,   // Left
        12, 13, 14,  14, 15, 12,  // Right
        16, 17, 18,  18, 19, 16,  // Top
        20, 21, 22,  22, 23, 20   // Bottom
    };
    
    mesh.SetVertices(vertices);
    mesh.SetIndices(indices);
    
    return mesh;
}

Mesh Mesh::CreateSphere(float radius, int segments) {
    Mesh mesh;
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * 3.14159f / segments;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);
        
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * 3.14159f / segments;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);
            
            Vector3 position(
                radius * sinTheta * cosPhi,
                radius * cosTheta,
                radius * sinTheta * sinPhi
            );
            
            Vector3 normal = position.Normalized();
            Vector3 texCoords(static_cast<float>(lon) / segments, static_cast<float>(lat) / segments, 0);
            
            vertices.emplace_back(position, normal, texCoords);
        }
    }
    
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int current = lat * (segments + 1) + lon;
            int next = current + segments + 1;
            
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    
    mesh.SetVertices(vertices);
    mesh.SetIndices(indices);
    
    return mesh;
}

Mesh Mesh::CreatePlane(float width, float height) {
    Mesh mesh;
    
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;
    
    std::vector<Vertex> vertices = {
        Vertex(Vector3(-halfWidth, 0, -halfHeight), Vector3::Up, Vector3(0, 0, 0)),
        Vertex(Vector3( halfWidth, 0, -halfHeight), Vector3::Up, Vector3(1, 0, 0)),
        Vertex(Vector3( halfWidth, 0,  halfHeight), Vector3::Up, Vector3(1, 1, 0)),
        Vertex(Vector3(-halfWidth, 0,  halfHeight), Vector3::Up, Vector3(0, 1, 0))
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };
    
    mesh.SetVertices(vertices);
    mesh.SetIndices(indices);
    
    return mesh;
}

}
