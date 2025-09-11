#pragma once

#include <string>

namespace GameEngine {
    class FrameCapture {
    public:
        static bool SaveDefaultFramebufferPNG(int width, int height, const std::string& filename);
    };
}
