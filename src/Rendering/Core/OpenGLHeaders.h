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

// Include OpenGL headers in the correct order
#include <GL/gl.h>
#include <GL/glext.h>

// Additional OpenGL utilities if needed
#ifdef _WIN32
    // Windows-specific OpenGL extensions can be added here if needed
#endif
