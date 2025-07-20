add_library(Rendering STATIC
    Renderer.cpp
    OpenGL/OpenGLRenderer.cpp
    Shaders/Shader.cpp
    Meshes/Mesh.cpp
    Core/Buffer.cpp
    Core/Texture.cpp
    Core/FrameBuffer.cpp
    Pipelines/DeferredRenderPipeline.cpp
    Pipelines/ForwardRenderPipeline.cpp
    Pipelines/RaytracingPipeline.cpp
    PostProcessing/PostProcessingStack.cpp
    Materials/Material.cpp
    Lighting/Light.cpp
    RenderManager.cpp
)

target_include_directories(Rendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Rendering PUBLIC
    Core
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLM_LIBRARIES}
    OpenGL::GL
)

# Platform-specific OpenGL extension loading configuration
if(WIN32)
    # Windows: glad handles static linking automatically
    target_compile_definitions(Rendering PRIVATE GLAD_GLAPI_EXPORT)
else()
    # Linux: Define GLEW_STATIC if using static GLEW
    if(GLEW_STATIC_LIBRARIES)
        target_compile_definitions(Rendering PRIVATE GLEW_STATIC)
    endif()
endif()

target_include_directories(Rendering PUBLIC
    ${GLEW_INCLUDE_DIRS}
)

target_compile_features(Rendering PUBLIC cxx_std_20)
