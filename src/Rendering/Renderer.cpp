#include "Renderer.h"
#include "OpenGL/OpenGLRenderer.h"
#include "../Core/Logging/Logger.h"

namespace GameEngine {

RendererAPI Renderer::s_api = RendererAPI::OpenGL;

std::unique_ptr<Renderer> Renderer::Create(RendererAPI api) {
    s_api = api;
    
    switch (api) {
        case RendererAPI::OpenGL:
            Logger::Info("Creating OpenGL Renderer");
            return std::make_unique<OpenGLRenderer>();
        case RendererAPI::DirectX11:
            Logger::Error("DirectX11 renderer not implemented yet");
            break;
        case RendererAPI::DirectX12:
            Logger::Error("DirectX12 renderer not implemented yet");
            break;
        case RendererAPI::Vulkan:
            Logger::Error("Vulkan renderer not implemented yet");
            break;
        default:
            Logger::Error("Unknown renderer API");
            break;
    }
    
    return nullptr;
}

}
