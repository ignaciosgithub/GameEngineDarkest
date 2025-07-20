add_library(Rendering STATIC
    Renderer.cpp
    OpenGL/OpenGLRenderer.cpp
    Shaders/Shader.cpp
    Meshes/Mesh.cpp
    Core/Buffer.cpp
    Core/Texture.cpp
    Core/FrameBuffer.cpp
    Pipelines/DeferredRenderPipeline.cpp
    RenderManager.cpp
)

target_include_directories(Rendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Rendering PUBLIC
    Core
    ${GLFW3_LIBRARIES}
    glm::glm
    GL
)

target_compile_features(Rendering PUBLIC cxx_std_20)
