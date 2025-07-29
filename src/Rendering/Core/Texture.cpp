#include "Texture.h"
#include "../../Core/Logging/Logger.h"
#include "OpenGLHeaders.h"

namespace GameEngine {

Texture::Texture() : m_textureID(0), m_width(0), m_height(0), m_format(TextureFormat::RGBA8) {
    glGenTextures(1, &m_textureID);
    Logger::Info("Texture created with ID: " + std::to_string(m_textureID));
}

Texture::~Texture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
    Logger::Info("Texture destroyed");
}

bool Texture::LoadFromFile(const std::string& path) {
    Logger::Info("Texture loading from file not implemented yet: " + path);
    CreateEmpty(256, 256, TextureFormat::RGBA8);
    return true;
}

void Texture::CreateEmpty(int width, int height, TextureFormat format) {
    m_width = width;
    m_height = height;
    m_format = format;
    
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    
    GLenum internalFormat = GetGLInternalFormat(format);
    GLenum dataFormat = GetGLFormat(format);
    GLenum dataType = GetGLType(format);
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, dataType, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    Logger::Info("Texture created empty: " + std::to_string(width) + "x" + std::to_string(height) + " with ID: " + std::to_string(m_textureID));
}

void Texture::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    Logger::Debug("Texture bound to slot " + std::to_string(slot) + " with ID: " + std::to_string(m_textureID));
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
    Logger::Debug("Texture unbound");
}

void Texture::SetFilter(TextureFilter minFilter, TextureFilter magFilter) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetGLFilter(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFilter(magFilter));
    glBindTexture(GL_TEXTURE_2D, 0);
    Logger::Debug("Texture filter set");
}

void Texture::SetWrap(TextureWrap wrapS, TextureWrap wrapT) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrap(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrap(wrapT));
    glBindTexture(GL_TEXTURE_2D, 0);
    Logger::Debug("Texture wrap set");
}

unsigned int Texture::GetGLFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB8:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            return GL_RGB;
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            return GL_RGBA;
        case TextureFormat::Depth24:
        case TextureFormat::Depth32F:
            return GL_DEPTH_COMPONENT;
        default:
            return GL_RGBA;
    }
}

unsigned int Texture::GetGLInternalFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB8: return GL_RGB8;
        case TextureFormat::RGBA8: return GL_RGBA8;
        case TextureFormat::RGB16F: return GL_RGB16F;
        case TextureFormat::RGBA16F: return GL_RGBA16F;
        case TextureFormat::RGB32F: return GL_RGB32F;
        case TextureFormat::RGBA32F: return GL_RGBA32F;
        case TextureFormat::Depth24: return GL_DEPTH_COMPONENT24;
        case TextureFormat::Depth32F: return GL_DEPTH_COMPONENT32F;
        default: return GL_RGBA8;
    }
}

unsigned int Texture::GetGLType(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGB8:
        case TextureFormat::RGBA8:
            return GL_UNSIGNED_BYTE;
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::Depth32F:
            return GL_FLOAT;
        case TextureFormat::Depth24:
            return GL_UNSIGNED_INT;
        default:
            return GL_UNSIGNED_BYTE;
    }
}

unsigned int Texture::GetGLFilter(TextureFilter filter) const {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        default: return GL_LINEAR;
    }
}

unsigned int Texture::GetGLWrap(TextureWrap wrap) const {
    switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        default: return GL_CLAMP_TO_EDGE;
    }
}

}
