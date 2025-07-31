add_library(Rendering STATIC
    Renderer.cpp
    OpenGL/OpenGLRenderer.cpp
    Shaders/Shader.cpp
    Meshes/Mesh.cpp
    Loaders/OBJLoader.cpp
    Core/Buffer.cpp
    Core/Texture.cpp
    Core/FrameBuffer.cpp
    Pipelines/DeferredRenderPipeline.cpp
    Pipelines/ForwardRenderPipeline.cpp
    Pipelines/RaytracingPipeline.cpp
    PostProcessing/PostProcessingStack.cpp
    Materials/Material.cpp
    Lighting/Light.cpp
    Lighting/LightManager.cpp
    Lighting/LightOcclusion.cpp
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

target_include_directories(Rendering PUBLIC
    ${GLEW_INCLUDE_DIRS}
)

target_compile_features(Rendering PUBLIC cxx_std_20)
