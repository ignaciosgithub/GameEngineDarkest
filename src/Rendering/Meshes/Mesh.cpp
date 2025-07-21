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
        Logger::Warning("Mesh not uploaded or vertex array not available");
        return;
    }
    
    Bind();
    
    if (m_indexBuffer && !m_indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
        Logger::Debug("Drawing mesh with " + std::to_string(m_indices.size()) + " indices");
    } else if (!m_vertices.empty()) {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
        Logger::Debug("Drawing mesh with " + std::to_string(m_vertices.size()) + " vertices");
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
    
    std::vector<Vertex> vertices = {
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 0.0f)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f)),
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 1.0f)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(1.0f, 0.5f, 0.0f)),
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.5f, 0.0f, 1.0f)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)),
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.5f)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.8f, 0.2f, 0.6f)),
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.2f, 0.8f, 0.4f)),
        
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.9f, 0.1f, 0.7f)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.3f, 0.7f, 0.9f)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.6f, 0.4f, 0.2f)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.1f, 0.9f, 0.3f)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.7f, 0.3f, 0.8f)),
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.4f, 0.6f, 0.1f)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.8f, 0.8f, 0.2f)),
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.2f, 0.2f, 0.8f)),
        
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.5f, 0.9f, 0.1f)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.9f, 0.5f, 0.7f)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.1f, 0.7f, 0.5f)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.7f, 0.1f, 0.9f))
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    
    mesh.SetVertices(vertices);
    mesh.SetIndices(indices);
    
    return mesh;
}

Mesh Mesh::CreateSphere(float radius, int /*segments*/) {
    return CreateCube(radius * 2.0f);
}

}
