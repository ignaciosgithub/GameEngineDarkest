#pragma once
#include "OpenGLHeaders.h"
#include "../../Core/Logging/Logger.h"
#include <cstdlib>

namespace GameEngine {

inline void EnableGLDebug() {
#ifdef GL_KHR_debug
    static bool installed = false;
    if (installed) return;
    const char* flag = std::getenv("GE_GL_KHR_DEBUG");
    if (!flag || std::string(flag) != "1") return;
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*) {
            const char* src =
                source == GL_DEBUG_SOURCE_API ? "API" :
                source == GL_DEBUG_SOURCE_WINDOW_SYSTEM ? "WINDOW_SYS" :
                source == GL_DEBUG_SOURCE_SHADER_COMPILER ? "SHADER" :
                source == GL_DEBUG_SOURCE_THIRD_PARTY ? "3RD_PARTY" :
                source == GL_DEBUG_SOURCE_APPLICATION ? "APP" : "OTHER";
            const char* typ =
                type == GL_DEBUG_TYPE_ERROR ? "ERROR" :
                type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ? "DEPRECATED" :
                type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR ? "UNDEFINED" :
                type == GL_DEBUG_TYPE_PORTABILITY ? "PORTABILITY" :
                type == GL_DEBUG_TYPE_PERFORMANCE ? "PERFORMANCE" :
                type == GL_DEBUG_TYPE_MARKER ? "MARKER" :
                type == GL_DEBUG_TYPE_PUSH_GROUP ? "PUSH" :
                type == GL_DEBUG_TYPE_POP_GROUP ? "POP" : "OTHER";
            const char* sev =
                severity == GL_DEBUG_SEVERITY_HIGH ? "HIGH" :
                severity == GL_DEBUG_SEVERITY_MEDIUM ? "MED" :
                severity == GL_DEBUG_SEVERITY_LOW ? "LOW" : "NOTIF";
            GameEngine::Logger::Error(std::string("[GL] src=") + src + " type=" + typ + " sev=" + sev + " id=" + std::to_string(id) + " msg=" + message);
        }, nullptr);
        installed = true;
        GameEngine::Logger::Info("KHR_debug enabled");
    }
#endif
}

} // namespace GameEngine
