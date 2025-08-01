#pragma once

#include <vector>
#include <memory>
#include <string>
#include "../../Core/Math/Vector3.h"
#include "../Core/Buffer.h"

namespace GameEngine {
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector3 color; // Using Vector3 for RGB color data
        
        Vertex() = default;
        Vertex(const Vector3& pos, const Vector3& norm = Vector3::Up, const Vector3& col = Vector3::Zero)
            : position(pos), normal(norm), color(col) {}
    };
    
    class Mesh {
    public:
        Mesh();
        ~Mesh();
        
        // Delete copy constructor and assignment operator due to unique_ptr members
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        
        // Enable move semantics
        Mesh(Mesh&&) = default;
        Mesh& operator=(Mesh&&) = default;
        
        void SetVertices(const std::vector<Vertex>& vertices);
        void SetIndices(const std::vector<unsigned int>& indices);
        
        void Upload();
        void Bind() const;
        void Unbind() const;
        void Draw() const;
        unsigned int GetIndexCount() const;
        bool IsUploaded() const { return m_uploaded; }
        
        const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        const std::vector<unsigned int>& GetIndices() const { return m_indices; }
        
        // Static mesh creation helpers
        static Mesh CreateCube(float size = 1.0f);
        static Mesh CreateSphere(float radius = 1.0f, int segments = 32);
        static Mesh CreatePlane(float width = 1.0f, float height = 1.0f);
        
        // File loading
        static Mesh LoadFromOBJ(const std::string& filepath);
        
    private:
        void CleanupBuffers();
        
        std::vector<Vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        
        std::unique_ptr<VertexArray> m_vertexArray;
        std::unique_ptr<Buffer> m_vertexBuffer;
        std::unique_ptr<Buffer> m_indexBuffer;
        
        bool m_uploaded = false;
    };
}
