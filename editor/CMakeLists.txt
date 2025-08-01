# Editor executable
add_executable(GameEngineEditor main.cpp)

# Link against the Core library and other dependencies
target_link_libraries(GameEngineEditor 
    Core
    Rendering
    Physics
    Physics2D
    UI
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLM_LIBRARIES}
    ${ASSIMP_LIBRARIES}
    ${OPENAL_LIBRARIES}
    imgui
)

# Set output directory
set_target_properties(GameEngineEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy resources if needed
if(EXISTS ${CMAKE_SOURCE_DIR}/resources)
    add_custom_command(TARGET GameEngineEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:GameEngineEditor>/resources
    )
endif()
