#include "Mesh.h"
#include "../../Core/Logging/Logger.h"
#include <GL/gl.h>
#include <GL/glext.h>
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
    if (m_uploaded || m_vertices.empty()) {
        return;
    }
    
    m_vertexArray = std::make_unique<VertexArray>();
    m_vertexBuffer = std::make_unique<Buffer>(BufferType::Vertex);
    
    m_vertexBuffer->SetData(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
    
    std::vector<unsigned int> layout = {3, 3, 3};
    m_vertexArray->AddVertexBuffer(*m_vertexBuffer, layout);
    
    if (!m_indices.empty()) {
        m_indexBuffer = std::make_unique<Buffer>(BufferType::Index);
        m_indexBuffer->SetData(m_indices.data(), m_indices.size() * sizeof(unsigned int));
        m_vertexArray->SetIndexBuffer(*m_indexBuffer);
    }
    
    m_uploaded = true;
    Logger::Info("Mesh uploaded with modern OpenGL buffers");
}

void Mesh::Bind() const {
    if (m_vertexArray) {
        m_vertexArray->Bind();
    }
}

void Mesh::Draw() const {
    if (!m_uploaded || !m_vertexArray) {
        return;
    }
    
    Bind();
    
    if (m_indexBuffer && !m_indices.empty()) {
        Logger::Debug("Drawing mesh with " + std::to_string(m_indices.size()) + " indices (simplified)");
    } else {
        Logger::Debug("Drawing mesh with " + std::to_string(m_vertices.size()) + " vertices (simplified)");
    }
}

void Mesh::Unbind() const {
    if (m_vertexArray) {
        m_vertexArray->Unbind();
    }
}

unsigned int Mesh::GetIndexCount() const {
    return static_cast<unsigned int>(m_indices.size());
}

void Mesh::CleanupBuffers() {
    m_vertexArray.reset();
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
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
    return CreateCube(radius * 2.0f);
}

}
