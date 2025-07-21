#include "Buffer.h"
#include "../../Core/Logging/Logger.h"
#include "OpenGLHeaders.h"

namespace GameEngine {

Buffer::Buffer(BufferType type, BufferUsage usage) 
    : m_type(type), m_usage(usage) {
    glGenBuffers(1, &m_bufferID);
    Logger::Info("Buffer created with ID: " + std::to_string(m_bufferID));
}

Buffer::~Buffer() {
    if (m_bufferID != 0) {
        glDeleteBuffers(1, &m_bufferID);
        Logger::Info("Buffer destroyed");
    }
}

void Buffer::SetData(const void* data, size_t size) {
    m_size = size;
    Bind();
    glBufferData(GetGLBufferType(), size, data, GetGLUsage());
    Logger::Info("Buffer data set, size: " + std::to_string(size));
}

void Buffer::SetSubData(const void* data, size_t size, size_t offset) {
    Bind();
    glBufferSubData(GetGLBufferType(), offset, size, data);
    Logger::Info("Buffer sub-data set");
}

void Buffer::Bind() const {
    glBindBuffer(GetGLBufferType(), m_bufferID);
}

void Buffer::Unbind() const {
    glBindBuffer(GetGLBufferType(), 0);
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

VertexArray::VertexArray() : m_indexCount(0), m_vertexBufferIndex(0) {
    glGenVertexArrays(1, &m_arrayID);
    Logger::Info("VertexArray created with ID: " + std::to_string(m_arrayID));
}

VertexArray::~VertexArray() {
    if (m_arrayID != 0) {
        glDeleteVertexArrays(1, &m_arrayID);
        Logger::Info("VertexArray destroyed");
    }
}

void VertexArray::Bind() const {
    glBindVertexArray(m_arrayID);
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const Buffer& vertexBuffer, const std::vector<unsigned int>& layout) {
    Bind();
    vertexBuffer.Bind();
    
    size_t offset = 0;
    size_t stride = 0;
    for (unsigned int count : layout) {
        stride += count * sizeof(float);
    }
    
    for (size_t i = 0; i < layout.size(); ++i) {
        glVertexAttribPointer(m_vertexBufferIndex, layout[i], GL_FLOAT, GL_FALSE, stride, (void*)offset);
        glEnableVertexAttribArray(m_vertexBufferIndex);
        offset += layout[i] * sizeof(float);
        m_vertexBufferIndex++;
    }
    
    Logger::Info("VertexArray buffer added with " + std::to_string(layout.size()) + " attributes");
}

void VertexArray::SetIndexBuffer(const Buffer& indexBuffer) {
    Logger::Info("VertexArray index buffer set");
    m_indexCount = static_cast<unsigned int>(indexBuffer.GetSize() / sizeof(unsigned int));
}

}
