#include "Buffer.h"
#include "../../Core/Logging/Logger.h"
#include <GL/gl.h>
#include <GL/glext.h>

namespace GameEngine {

Buffer::Buffer(BufferType type, BufferUsage usage) 
    : m_bufferID(1), m_type(type), m_usage(usage) {
    Logger::Info("Buffer created (simplified for compatibility)");
}

Buffer::~Buffer() {
    Logger::Info("Buffer destroyed");
}

void Buffer::SetData(const void* /*data*/, size_t size) {
    m_size = size;
    Logger::Info("Buffer data set, size: " + std::to_string(size));
}

void Buffer::SetSubData(const void* /*data*/, size_t /*size*/, size_t /*offset*/) {
    Logger::Info("Buffer sub-data set");
}

void Buffer::Bind() const {
    Logger::Debug("Buffer bind (simplified)");
}

void Buffer::Unbind() const {
    Logger::Debug("Buffer unbind (simplified)");
}

unsigned int Buffer::GetGLBufferType() const {
    switch (m_type) {
        case BufferType::Vertex: return GL_ARRAY_BUFFER;
        case BufferType::Index: return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::Uniform: return GL_UNIFORM_BUFFER;
        default: return GL_ARRAY_BUFFER;
    }
}

unsigned int Buffer::GetGLUsage() const {
    switch (m_usage) {
        case BufferUsage::Static: return GL_STATIC_DRAW;
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream: return GL_STREAM_DRAW;
        default: return GL_STATIC_DRAW;
    }
}

VertexArray::VertexArray() : m_arrayID(1), m_indexCount(0), m_vertexBufferIndex(0) {
    Logger::Info("VertexArray created (simplified for compatibility)");
}

VertexArray::~VertexArray() {
    Logger::Info("VertexArray destroyed");
}

void VertexArray::Bind() const {
    Logger::Debug("VertexArray bind (simplified)");
}

void VertexArray::Unbind() const {
    Logger::Debug("VertexArray unbind (simplified)");
}

void VertexArray::AddVertexBuffer(const Buffer& /*vertexBuffer*/, const std::vector<unsigned int>& layout) {
    Logger::Info("VertexArray buffer added with " + std::to_string(layout.size()) + " attributes");
    m_vertexBufferIndex += static_cast<unsigned int>(layout.size());
}

void VertexArray::SetIndexBuffer(const Buffer& indexBuffer) {
    Logger::Info("VertexArray index buffer set");
    m_indexCount = static_cast<unsigned int>(indexBuffer.GetSize() / sizeof(unsigned int));
}

}
