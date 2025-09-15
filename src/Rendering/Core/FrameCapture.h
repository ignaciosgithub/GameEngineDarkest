#pragma once
#include <string>

namespace GameEngine {
    class Texture;

    class FrameCapture {
    public:
        static bool SaveDefaultFramebufferPNG(int width, int height, const std::string& filename);
        static bool SaveTexturePNG(Texture* texture, int width, int height, const std::string& filename);
    };
}
