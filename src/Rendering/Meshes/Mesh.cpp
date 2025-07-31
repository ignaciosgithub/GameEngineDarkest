#include "Mesh.h"
#include "../Loaders/OBJLoader.h"
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
        Logger::Debug("Mesh upload skipped - uploaded: " + std::to_string(m_uploaded) + ", vertices empty: " + std::to_string(m_vertices.empty()));
        return;
    }
    
    Logger::Debug("Starting mesh upload with " + std::to_string(m_vertices.size()) + " vertices");
    Logger::Debug("First vertex - pos: (" + std::to_string(m_vertices[0].position.x) + ", " + std::to_string(m_vertices[0].position.y) + ", " + std::to_string(m_vertices[0].position.z) + ")");
    Logger::Debug("First vertex - color: (" + std::to_string(m_vertices[0].color.x) + ", " + std::to_string(m_vertices[0].color.y) + ", " + std::to_string(m_vertices[0].color.z) + ")");
    
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
    
    Logger::Debug("Mesh::Draw() - Starting draw call");
    
    Bind();
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        Logger::Error("OpenGL error after VAO bind: " + std::to_string(error));
    }
    
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    Logger::Debug("Current shader program: " + std::to_string(currentProgram));
    
    if (currentProgram == 0) {
        Logger::Error("No shader program bound during mesh draw!");
        return;
    }
    
    if (m_indexBuffer && !m_indices.empty()) {
        Logger::Debug("Drawing mesh with " + std::to_string(m_indices.size()) + " indices using glDrawElements");
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            Logger::Error("OpenGL error after glDrawElements: " + std::to_string(error));
        } else {
            Logger::Debug("glDrawElements completed successfully");
        }
    } else if (!m_vertices.empty()) {
        Logger::Debug("Drawing mesh with " + std::to_string(m_vertices.size()) + " vertices using glDrawArrays");
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            Logger::Error("OpenGL error after glDrawArrays: " + std::to_string(error));
        } else {
            Logger::Debug("glDrawArrays completed successfully");
        }
    }
    
    Logger::Debug("Mesh::Draw() - Draw call completed");
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
    Logger::Debug("Creating cube mesh with size: " + std::to_string(size));
    Mesh mesh;
    
    float halfSize = size * 0.5f;
    
    std::vector<Vertex> vertices = {
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(0.0f, 0.0f, 1.0f), Vector3(1.0f, 0.0f, 0.0f)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),
        
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(1.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f)),
        
        Vertex(Vector3(-halfSize, -halfSize, -halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(1.0f, 0.0f, 1.0f)),
        Vertex(Vector3( halfSize, -halfSize, -halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(1.0f, 0.0f, 1.0f)),
        Vertex(Vector3( halfSize, -halfSize,  halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(1.0f, 0.0f, 1.0f)),
        Vertex(Vector3(-halfSize, -halfSize,  halfSize), Vector3(0.0f, -1.0f, 0.0f), Vector3(1.0f, 0.0f, 1.0f)),
        
        Vertex(Vector3(-halfSize,  halfSize, -halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 1.0f)),
        Vertex(Vector3( halfSize,  halfSize, -halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 1.0f)),
        Vertex(Vector3( halfSize,  halfSize,  halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 1.0f)),
        Vertex(Vector3(-halfSize,  halfSize,  halfSize), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 1.0f))
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
    
    Logger::Debug("Cube mesh created with " + std::to_string(vertices.size()) + " vertices and " + std::to_string(indices.size()) + " indices");
    return mesh;
}

Mesh Mesh::CreateSphere(float radius, int /*segments*/) {
    return CreateCube(radius * 2.0f);
}

Mesh Mesh::CreatePlane(float width, float height) {
    Mesh mesh;
    
    std::vector<Vertex> vertices = {
        {{-width/2, 0, -height/2}, {0, 1, 0}, {0.8f, 0.8f, 0.8f}}, // Bottom-left
        {{ width/2, 0, -height/2}, {0, 1, 0}, {0.8f, 0.8f, 0.8f}}, // Bottom-right
        {{ width/2, 0,  height/2}, {0, 1, 0}, {0.8f, 0.8f, 0.8f}}, // Top-right
        {{-width/2, 0,  height/2}, {0, 1, 0}, {0.8f, 0.8f, 0.8f}}  // Top-left
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    mesh.SetVertices(vertices);
    mesh.SetIndices(indices);
    mesh.Upload();
    
    Logger::Debug("Created plane mesh with " + std::to_string(vertices.size()) + " vertices");
    return mesh;
}

Mesh Mesh::LoadFromOBJ(const std::string& filepath) {
    return OBJLoader::LoadFromFile(filepath);
}

}
