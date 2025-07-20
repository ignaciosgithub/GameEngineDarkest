#pragma once

#include <vector>
#include <cstddef>

namespace GameEngine {
    enum class BufferType {
        Vertex,
        Index,
        Uniform
    };

    enum class BufferUsage {
        Static,
        Dynamic,
        Stream
    };

    class Buffer {
    public:
        Buffer(BufferType type, BufferUsage usage = BufferUsage::Static);
        ~Buffer();

        void SetData(const void* data, size_t size);
        void SetSubData(const void* data, size_t size, size_t offset);
        
        void Bind() const;
        void Unbind() const;
        
        unsigned int GetID() const { return m_bufferID; }
        size_t GetSize() const { return m_size; }
        BufferType GetType() const { return m_type; }

    private:
        unsigned int m_bufferID = 0;
        BufferType m_type;
        BufferUsage m_usage;
        size_t m_size = 0;
        
        unsigned int GetGLBufferType() const;
        unsigned int GetGLUsage() const;
    };

    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        void Bind() const;
        void Unbind() const;
        
        void AddVertexBuffer(const Buffer& vertexBuffer, const std::vector<unsigned int>& layout);
        void SetIndexBuffer(const Buffer& indexBuffer);
        
        unsigned int GetID() const { return m_arrayID; }
        unsigned int GetIndexCount() const { return m_indexCount; }

    private:
        unsigned int m_arrayID = 0;
        unsigned int m_indexCount = 0;
        unsigned int m_vertexBufferIndex = 0;
    };
}
