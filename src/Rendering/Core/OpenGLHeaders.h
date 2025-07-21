#pragma once

// Windows-specific OpenGL header management to prevent WINGDIAPI/APIENTRY redefinition
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    // Undefine problematic macros that conflict with C++ code
    #undef near
    #undef far
    #undef min
    #undef max
#endif

// Platform-specific OpenGL extension loading
#ifdef _WIN32
    #include <glad/glad.h>
#else
    #include <GL/glew.h>
#endif

// Additional OpenGL utilities if needed
#ifdef _WIN32
    // Windows-specific OpenGL extensions can be added here if needed
#endif
