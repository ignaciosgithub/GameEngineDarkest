#pragma once

#include <vector>
#include "../../Core/Math/Vector3.h"

namespace GameEngine {
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector3 texCoords; // Using Vector3 for UV + potential third coordinate
        
        Vertex() = default;
        Vertex(const Vector3& pos, const Vector3& norm = Vector3::Up, const Vector3& tex = Vector3::Zero)
            : position(pos), normal(norm), texCoords(tex) {}
    };
    
    class Mesh {
    public:
        Mesh();
        ~Mesh();
        
        void SetVertices(const std::vector<Vertex>& vertices);
        void SetIndices(const std::vector<unsigned int>& indices);
        
        void Upload();
        void Bind() const;
        void Unbind() const;
        void Draw() const;
        unsigned int GetIndexCount() const;
        
        const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        const std::vector<unsigned int>& GetIndices() const { return m_indices; }
        
        // Static mesh creation helpers
        static Mesh CreateCube(float size = 1.0f);
        static Mesh CreateSphere(float radius = 1.0f, int segments = 32);
        static Mesh CreatePlane(float width = 1.0f, float height = 1.0f);
        
    private:
        void CleanupBuffers();
        
        std::vector<Vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        
        unsigned int m_VAO = 0;
        unsigned int m_VBO = 0;
        unsigned int m_EBO = 0;
        
        bool m_uploaded = false;
    };
}
