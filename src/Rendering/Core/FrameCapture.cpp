#include "Texture.h"

#include "FrameCapture.h"
#include "OpenGLHeaders.h"
#include <vector>
#include <cstring>
#include "../../Core/Logging/Logger.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../Core/stb_image_write.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace GameEngine {

static void FlipImageVertical(unsigned char* data, int width, int height, int channels) {
    int stride = width * channels;
    std::vector<unsigned char> row(stride);
    for (int y = 0; y < height / 2; ++y) {
        unsigned char* top = data + y * stride;
        unsigned char* bottom = data + (height - 1 - y) * stride;
        std::memcpy(row.data(), top, stride);
        std::memcpy(top, bottom, stride);
        std::memcpy(bottom, row.data(), stride);
    }
}

bool FrameCapture::SaveDefaultFramebufferPNG(int width, int height, const std::string& filename) {
    if (width <= 0 || height <= 0) {
        Logger::Error("FrameCapture: invalid dimensions");
        return false;
    }
    std::vector<unsigned char> pixels(width * height * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    FlipImageVertical(pixels.data(), width, height, 4);
    int ok = stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);
    if (!ok) {
        Logger::Error("FrameCapture: failed to write png: " + filename);
        return false;
    }
    Logger::Info("FrameCapture: wrote " + filename);
    return true;
}
bool FrameCapture::SaveTexturePNG(Texture* texture, int width, int height, const std::string& filename) {
    if (!texture || width <= 0 || height <= 0) {
        Logger::Error("FrameCapture: invalid texture or dimensions");
        return false;
    }
    std::vector<unsigned char> pixels(width * height * 4);
    glBindTexture(GL_TEXTURE_2D, texture->GetID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    FlipImageVertical(pixels.data(), width, height, 4);
    int ok = stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);
    if (!ok) {
        Logger::Error("FrameCapture: failed to write texture png: " + filename);
        return false;
    }
    Logger::Info("FrameCapture: wrote " + filename);
    return true;
}


}
