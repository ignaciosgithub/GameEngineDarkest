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
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        Logger::Error("OpenGL error in VertexArray::Bind for VAO " + std::to_string(m_arrayID) + ": " + std::to_string(err));
    } else {
        Logger::Debug("VertexArray::Bind VAO " + std::to_string(m_arrayID));
    }
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        Logger::Error("OpenGL error in VertexArray::Unbind: " + std::to_string(err));
    } else {
        Logger::Debug("VertexArray::Unbind");
    }
}

void VertexArray::AddVertexBuffer(const Buffer& vertexBuffer, const std::vector<unsigned int>& layout) {
    Bind();
    vertexBuffer.Bind();
    
    m_vertexBufferIndex = 0;
    
    size_t offset = 0;
    size_t stride = 0;
    for (unsigned int count : layout) {
        stride += count * sizeof(float);
    }
    
    for (size_t i = 0; i < layout.size(); ++i) {
        Logger::Info("Setting vertex attribute " + std::to_string(m_vertexBufferIndex) + " with " + std::to_string(layout[i]) + " components, stride=" + std::to_string(stride) + ", offset=" + std::to_string(offset));
        glVertexAttribPointer(m_vertexBufferIndex, static_cast<GLint>(layout[i]), GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)offset);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            Logger::Error("OpenGL error after glVertexAttribPointer: " + std::to_string(error));
        }
        glEnableVertexAttribArray(m_vertexBufferIndex);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            Logger::Error("OpenGL error after glEnableVertexAttribArray: " + std::to_string(error));
        }
        offset += layout[i] * sizeof(float);
        m_vertexBufferIndex++;
    }
    
    Logger::Info("VertexArray buffer added with " + std::to_string(layout.size()) + " attributes");
}

void VertexArray::SetIndexBuffer(const Buffer& indexBuffer) {
    Bind();
    indexBuffer.Bind();
    Logger::Info("VertexArray index buffer set");
    m_indexCount = static_cast<unsigned int>(indexBuffer.GetSize() / sizeof(unsigned int));
}

}
