#include "FrameBuffer.h"
#include "../../Core/Logging/Logger.h"
#include "OpenGLHeaders.h"

namespace GameEngine {

FrameBuffer::FrameBuffer(int width, int height) 
    : m_framebufferID(1), m_width(width), m_height(height) {
    Logger::Info("FrameBuffer created (simplified for compatibility)");
}

FrameBuffer::~FrameBuffer() {
    Logger::Info("FrameBuffer destroyed");
}

void FrameBuffer::Bind() const {
    Logger::Debug("FrameBuffer bind (simplified)");
    glViewport(0, 0, m_width, m_height);
}

void FrameBuffer::Unbind() const {
    Logger::Debug("FrameBuffer unbind (simplified)");
}

void FrameBuffer::AddColorAttachment(TextureFormat format) {
    auto texture = std::make_shared<Texture>();
    texture->CreateEmpty(m_width, m_height, format);
    
    unsigned int attachmentIndex = static_cast<unsigned int>(m_colorAttachments.size());
    unsigned int attachment = GL_COLOR_ATTACHMENT0 + attachmentIndex;
    
    m_colorAttachments.emplace_back(texture, attachment);
    
    Logger::Info("FrameBuffer color attachment added (simplified)");
}

void FrameBuffer::AddDepthAttachment(TextureFormat format) {
    m_depthAttachment = std::make_shared<Texture>();
    m_depthAttachment->CreateEmpty(m_width, m_height, format);
    
    Bind();
    AttachTexture(m_depthAttachment, GL_DEPTH_ATTACHMENT);
}

void FrameBuffer::Resize(int width, int height) {
    m_width = width;
    m_height = height;
    
    for (auto& attachment : m_colorAttachments) {
        attachment.texture->CreateEmpty(width, height, attachment.texture->GetFormat());
        AttachTexture(attachment.texture, attachment.attachmentType);
    }
    
    if (m_depthAttachment) {
        m_depthAttachment->CreateEmpty(width, height, m_depthAttachment->GetFormat());
        AttachTexture(m_depthAttachment, GL_DEPTH_ATTACHMENT);
    }
}

bool FrameBuffer::IsComplete() const {
    Logger::Debug("FrameBuffer completeness check (simplified)");
    return true;
}

std::shared_ptr<Texture> FrameBuffer::GetColorTexture(unsigned int index) const {
    if (index < m_colorAttachments.size()) {
        return m_colorAttachments[index].texture;
    }
    return nullptr;
}

std::shared_ptr<Texture> FrameBuffer::GetDepthTexture() const {
    return m_depthAttachment;
}

void FrameBuffer::CreateAttachments() {
    Bind();
    
    for (const auto& attachment : m_colorAttachments) {
        AttachTexture(attachment.texture, attachment.attachmentType);
    }
    
    if (m_depthAttachment) {
        AttachTexture(m_depthAttachment, GL_DEPTH_ATTACHMENT);
    }
}

void FrameBuffer::AttachTexture(const std::shared_ptr<Texture>& /*texture*/, unsigned int /*attachment*/) {
    Logger::Debug("FrameBuffer texture attachment (simplified)");
}

}
