#include "FrameBuffer.h"
#include "../../Core/Logging/Logger.h"
#include "OpenGLHeaders.h"

namespace GameEngine {

FrameBuffer::FrameBuffer(int width, int height) 
    : m_width(width), m_height(height) {
    glGenFramebuffers(1, &m_framebufferID);
    Logger::Info("FrameBuffer created with ID: " + std::to_string(m_framebufferID));
}

FrameBuffer::~FrameBuffer() {
    if (m_framebufferID != 0) {
        glDeleteFramebuffers(1, &m_framebufferID);
    }
    Logger::Info("FrameBuffer destroyed");
}

void FrameBuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
    glViewport(0, 0, m_width, m_height);
    Logger::Debug("FrameBuffer bound with ID: " + std::to_string(m_framebufferID));
}

void FrameBuffer::Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Logger::Debug("FrameBuffer unbound");
}

void FrameBuffer::AddColorAttachment(TextureFormat format) {
    auto texture = std::make_shared<Texture>();
    texture->CreateEmpty(m_width, m_height, format);
    
    unsigned int attachmentIndex = static_cast<unsigned int>(m_colorAttachments.size());
    unsigned int attachment = GL_COLOR_ATTACHMENT0 + attachmentIndex;
    
    m_colorAttachments.emplace_back(texture, attachment);
    
    Bind();
    AttachTexture(texture, attachment);
    
    Logger::Info("FrameBuffer color attachment " + std::to_string(attachmentIndex) + " added");
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
    Bind();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool complete = (status == GL_FRAMEBUFFER_COMPLETE);
    if (!complete) {
        Logger::Error("FrameBuffer is not complete. Status: " + std::to_string(status));
    }
    Logger::Debug(std::string("FrameBuffer completeness check: ") + (complete ? "COMPLETE" : "INCOMPLETE"));
    return complete;
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
    
    std::vector<GLenum> drawBuffers;
    for (const auto& attachment : m_colorAttachments) {
        AttachTexture(attachment.texture, attachment.attachmentType);
        drawBuffers.push_back(attachment.attachmentType);
    }
    
    if (!drawBuffers.empty()) {
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }
    
    if (m_depthAttachment) {
        AttachTexture(m_depthAttachment, GL_DEPTH_ATTACHMENT);
    }
}

void FrameBuffer::AttachTexture(const std::shared_ptr<Texture>& texture, unsigned int attachment) {
    if (texture) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->GetID(), 0);
        Logger::Debug("Texture attached to framebuffer attachment: " + std::to_string(attachment));
    }
}

}
