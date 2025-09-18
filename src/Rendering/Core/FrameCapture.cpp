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

    auto fmt = texture->GetFormat();
    bool isFloat =
        fmt == TextureFormat::RGB16F || fmt == TextureFormat::RGBA16F ||
        fmt == TextureFormat::RGB32F || fmt == TextureFormat::RGBA32F ||
        fmt == TextureFormat::Depth32F;

    std::vector<unsigned char> out(width * height * 4);
    glBindTexture(GL_TEXTURE_2D, texture->GetID());

    if (isFloat) {
        std::vector<float> fpix(width * height * 4, 0.0f);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, fpix.data());
        for (size_t i = 0; i < fpix.size(); ++i) {
            float v = fpix[i];
            if (!std::isfinite(v)) v = 0.0f;
            v = std::max(0.0f, std::min(v, 1.0f));
            out[i] = static_cast<unsigned char>(v * 255.0f + 0.5f);
        }
    } else {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, out.data());
    }

    FlipImageVertical(out.data(), width, height, 4);
    int ok = stbi_write_png(filename.c_str(), width, height, 4, out.data(), width * 4);
    if (!ok) {
        Logger::Error("FrameCapture: failed to write texture png: " + filename);
        return false;
    }
    Logger::Info("FrameCapture: wrote " + filename);
    return true;
}


}
