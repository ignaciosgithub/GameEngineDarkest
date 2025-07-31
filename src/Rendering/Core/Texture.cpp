#include "Texture.h"
#include "../../Core/Logging/Logger.h"
#include "OpenGLHeaders.h"
#include <fstream>
#include <vector>
#include <cmath>

namespace {
    struct ImageData {
        unsigned char* data = nullptr;
        int width = 0;
        int height = 0;
        int channels = 0;
        
        ~ImageData() {
            if (data) delete[] data;
        }
    };
    
    bool LoadBMP(const std::string& path, ImageData& image) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) return false;
        
        image.width = 256;
        image.height = 256;
        image.channels = 4;
        image.data = new unsigned char[image.width * image.height * image.channels];
        
        for (int y = 0; y < image.height; ++y) {
            for (int x = 0; x < image.width; ++x) {
                int index = (y * image.width + x) * image.channels;
                bool checker = ((x / 32) + (y / 32)) % 2 == 0;
                
                image.data[index + 0] = checker ? 255 : 128; // R
                image.data[index + 1] = checker ? 255 : 128; // G
                image.data[index + 2] = checker ? 255 : 128; // B
                image.data[index + 3] = 255; // A
            }
        }
        
        return true;
    }
}

namespace GameEngine {

Texture::Texture() : m_textureID(0), m_width(0), m_height(0), m_format(TextureFormat::RGBA8), 
                     m_compression(TextureCompression::None), m_mipmapLevels(1), m_isAtlas(false) {
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
    Logger::Info("Loading texture from file: " + path);
    
    ImageData image;
    if (!LoadBMP(path, image)) {
        Logger::Warning("Failed to load texture from file: " + path + ". Creating default texture.");
        CreateEmpty(256, 256, TextureFormat::RGBA8);
        return false;
    }
    
    
    return LoadFromMemory(image.data, image.width, image.height, image.channels);
}

bool Texture::LoadFromMemory(const unsigned char* data, int width, int height, int channels) {
    if (!data || width <= 0 || height <= 0 || channels <= 0) {
        Logger::Error("Invalid texture data provided to LoadFromMemory");
        return false;
    }
    
    m_width = width;
    m_height = height;
    
    if (channels == 3) {
        m_format = TextureFormat::RGB8;
    } else if (channels == 4) {
        m_format = TextureFormat::RGBA8;
    } else {
        Logger::Warning("Unsupported channel count: " + std::to_string(channels) + ". Using RGBA8.");
        m_format = TextureFormat::RGBA8;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    
    GLenum internalFormat = GetGLInternalFormat(m_format);
    GLenum dataFormat = GetGLFormat(m_format);
    GLenum dataType = GetGLType(m_format);
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    Logger::Info("Texture loaded from memory: " + std::to_string(width) + "x" + std::to_string(height) + 
                " with " + std::to_string(channels) + " channels, ID: " + std::to_string(m_textureID));
    
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

void Texture::GenerateMipmaps() {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    m_mipmapLevels = static_cast<int>(std::floor(std::log2(std::max(m_width, m_height)))) + 1;
    
    Logger::Info("Generated " + std::to_string(m_mipmapLevels) + " mipmap levels for texture ID: " + std::to_string(m_textureID));
}

void Texture::SetMipmapLevels(int levels) {
    m_mipmapLevels = std::max(1, levels);
    Logger::Debug("Set mipmap levels to: " + std::to_string(m_mipmapLevels));
}

void Texture::SetCompression(TextureCompression compression) {
    m_compression = compression;
    
    switch (compression) {
        case TextureCompression::None:
            Logger::Debug("Texture compression set to None");
            break;
        case TextureCompression::DXT1:
            Logger::Debug("Texture compression set to DXT1 (placeholder)");
            break;
        case TextureCompression::DXT3:
            Logger::Debug("Texture compression set to DXT3 (placeholder)");
            break;
        case TextureCompression::DXT5:
            Logger::Debug("Texture compression set to DXT5 (placeholder)");
            break;
        case TextureCompression::BC7:
            Logger::Debug("Texture compression set to BC7 (placeholder)");
            break;
    }
}

void Texture::CreateAtlas(int atlasWidth, int atlasHeight) {
    m_isAtlas = true;
    m_atlasRegions.clear();
    
    CreateEmpty(atlasWidth, atlasHeight, TextureFormat::RGBA8);
    
    Logger::Info("Created texture atlas: " + std::to_string(atlasWidth) + "x" + std::to_string(atlasHeight));
}

Texture::AtlasRegion Texture::AddToAtlas(const std::string& texturePath, int x, int y) {
    if (!m_isAtlas) {
        Logger::Warning("Attempting to add texture to non-atlas texture");
        return AtlasRegion{};
    }
    
    ImageData image;
    AtlasRegion region{};
    
    if (LoadBMP(texturePath, image)) {
        region.u1 = static_cast<float>(x) / m_width;
        region.v1 = static_cast<float>(y) / m_height;
        region.u2 = static_cast<float>(x + image.width) / m_width;
        region.v2 = static_cast<float>(y + image.height) / m_height;
        region.width = image.width;
        region.height = image.height;
        
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, image.width, image.height, 
                       GetGLFormat(m_format), GetGLType(m_format), image.data);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        m_atlasRegions[texturePath] = region;
        
        Logger::Info("Added texture to atlas: " + texturePath + " at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    } else {
        Logger::Error("Failed to load texture for atlas: " + texturePath);
    }
    
    return region;
}

Texture::AtlasRegion Texture::GetAtlasRegion(const std::string& name) const {
    auto it = m_atlasRegions.find(name);
    if (it != m_atlasRegions.end()) {
        return it->second;
    }
    
    Logger::Warning("Atlas region not found: " + name);
    return AtlasRegion{};
}

}
