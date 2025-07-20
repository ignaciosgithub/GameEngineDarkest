#pragma once

#include <string>

namespace GameEngine {
    enum class TextureFormat {
        RGB8,
        RGBA8,
        RGB16F,
        RGBA16F,
        RGB32F,
        RGBA32F,
        Depth24,
        Depth32F
    };

    enum class TextureFilter {
        Nearest,
        Linear
    };

    enum class TextureWrap {
        Repeat,
        ClampToEdge,
        ClampToBorder
    };

    class Texture {
    public:
        Texture();
        ~Texture();

        bool LoadFromFile(const std::string& path);
        void CreateEmpty(int width, int height, TextureFormat format);
        
        void Bind(unsigned int slot = 0) const;
        void Unbind() const;
        
        void SetFilter(TextureFilter minFilter, TextureFilter magFilter);
        void SetWrap(TextureWrap wrapS, TextureWrap wrapT);
        
        unsigned int GetID() const { return m_textureID; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        TextureFormat GetFormat() const { return m_format; }

    private:
        unsigned int m_textureID = 0;
        int m_width = 0;
        int m_height = 0;
        TextureFormat m_format = TextureFormat::RGBA8;
        
        unsigned int GetGLFormat(TextureFormat format) const;
        unsigned int GetGLInternalFormat(TextureFormat format) const;
        unsigned int GetGLType(TextureFormat format) const;
        unsigned int GetGLFilter(TextureFilter filter) const;
        unsigned int GetGLWrap(TextureWrap wrap) const;
    };
}
