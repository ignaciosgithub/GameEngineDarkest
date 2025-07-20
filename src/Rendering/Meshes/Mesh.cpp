#include "Mesh.h"
#include "../../Core/Logging/Logger.h"
#include <GL/gl.h>
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
    Logger::Info("Mesh uploaded (simplified for demo)");
    m_uploaded = true;
}

void Mesh::Bind() const {
}

void Mesh::Draw() const {
}

void Mesh::Unbind() const {
}

unsigned int Mesh::GetIndexCount() const {
    return static_cast<unsigned int>(m_indices.size());
}

void Mesh::CleanupBuffers() {
    m_uploaded = false;
}

Mesh Mesh::CreateCube(float size) {
    Mesh mesh;
    
    float halfSize = size * 0.5f;
    
    std::vector<Vertex> vertices;
    vertices.push_back(Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f)));
    vertices.push_back(Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f)));
    vertices.push_back(Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 0.0f)));
    vertices.push_back(Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)));
    
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0
    };
    
    mesh.SetVertices(vertices);
    mesh.SetIndices(indices);
    
    return mesh;
}

Mesh Mesh::CreateSphere(float radius, int /*segments*/) {
    Mesh mesh;
    
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    return CreateCube(radius * 2.0f);
}

}
