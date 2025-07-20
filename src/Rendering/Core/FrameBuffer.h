#pragma once

#include "Texture.h"
#include <memory>
#include <vector>

namespace GameEngine {
    struct FrameBufferAttachment {
        std::shared_ptr<Texture> texture;
        unsigned int attachmentType;
        
        FrameBufferAttachment(std::shared_ptr<Texture> tex, unsigned int type)
            : texture(tex), attachmentType(type) {}
    };

    class FrameBuffer {
    public:
        FrameBuffer(int width, int height);
        ~FrameBuffer();

        void Bind() const;
        void Unbind() const;
        
        void AddColorAttachment(TextureFormat format);
        void AddDepthAttachment(TextureFormat format = TextureFormat::Depth24);
        
        void Resize(int width, int height);
        bool IsComplete() const;
        
        std::shared_ptr<Texture> GetColorTexture(unsigned int index = 0) const;
        std::shared_ptr<Texture> GetDepthTexture() const;
        
        unsigned int GetID() const { return m_framebufferID; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        unsigned int m_framebufferID = 0;
        int m_width;
        int m_height;
        
        std::vector<FrameBufferAttachment> m_colorAttachments;
        std::shared_ptr<Texture> m_depthAttachment;
        
        void CreateAttachments();
        void AttachTexture(const std::shared_ptr<Texture>& texture, unsigned int attachment);
    };
}
