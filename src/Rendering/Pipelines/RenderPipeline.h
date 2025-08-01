#pragma once

#include "../Core/FrameBuffer.h"
#include "../../Core/Math/Matrix4.h"
#include <memory>

namespace GameEngine {
    class World;
    class Renderer;
    
    struct RenderData {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        int viewportWidth;
        int viewportHeight;
    };

    class RenderPipeline {
    public:
        virtual ~RenderPipeline() = default;
        
        virtual bool Initialize(int width, int height) = 0;
        virtual void Shutdown() = 0;
        
        virtual void BeginFrame(const RenderData& renderData) = 0;
        virtual void Render(World* world) = 0;
        virtual void EndFrame() = 0;
        
        virtual void Resize(int width, int height) = 0;
        
        virtual std::shared_ptr<Texture> GetFinalTexture() const = 0;
        virtual std::shared_ptr<FrameBuffer> GetFramebuffer() const = 0;

    protected:
        int m_width = 0;
        int m_height = 0;
        RenderData m_renderData;
    };
}
