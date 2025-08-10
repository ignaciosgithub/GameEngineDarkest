#pragma once

#include <string>
#include <map>

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

    enum class TextureCompression {
        None,
        DXT1,
        DXT3,
        DXT5,
        BC7
    };

    class Texture {
    public:
        Texture();
        ~Texture();
        void CreateEmptyCubeDepth(int size, TextureFormat format);

        bool LoadFromFile(const std::string& path);
        bool LoadFromMemory(const unsigned char* data, int width, int height, int channels);
        void CreateEmpty(int width, int height, TextureFormat format);
        
        // Mipmap support
        void GenerateMipmaps();
        void SetMipmapLevels(int levels);
        
        // Compression support
        void SetCompression(TextureCompression compression);
        TextureCompression GetCompression() const { return m_compression; }
        
        void Bind(unsigned int slot = 0) const;
        void Unbind() const;
        
        void SetFilter(TextureFilter minFilter, TextureFilter magFilter);
        void SetWrap(TextureWrap wrapS, TextureWrap wrapT);
        
        unsigned int GetID() const { return m_textureID; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        TextureFormat GetFormat() const { return m_format; }
        int GetMipmapLevels() const { return m_mipmapLevels; }
        
        // Texture atlas support
        struct AtlasRegion {
            float u1, v1, u2, v2; // UV coordinates
            int width, height;     // Region dimensions
        };
        
        void CreateAtlas(int atlasWidth, int atlasHeight);
        AtlasRegion AddToAtlas(const std::string& texturePath, int x, int y);
        AtlasRegion GetAtlasRegion(const std::string& name) const;

    private:
        unsigned int m_textureID = 0;
        int m_width = 0;
        int m_height = 0;
        TextureFormat m_format = TextureFormat::RGBA8;
        TextureCompression m_compression = TextureCompression::None;
        int m_mipmapLevels = 1;
        
        // Atlas support
        bool m_isAtlas = false;
        std::map<std::string, AtlasRegion> m_atlasRegions;
        
        bool m_isCube = false;
        unsigned int GetGLFormat(TextureFormat format) const;
        unsigned int GetGLInternalFormat(TextureFormat format) const;
        unsigned int GetGLType(TextureFormat format) const;
        unsigned int GetGLFilter(TextureFilter filter) const;
        unsigned int GetGLWrap(TextureWrap wrap) const;
    };
}
