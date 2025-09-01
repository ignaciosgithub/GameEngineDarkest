add_library(UI STATIC
    ImGuiRenderer.cpp
    EngineUI.cpp
    Panels/SceneHierarchyPanel.cpp
    Panels/InspectorPanel.cpp
    Panels/ViewportPanel.cpp
    Panels/ProjectPanel.cpp
    Panels/ConsolePanel.cpp
    Panels/WorldSettingsPanel.cpp
)

target_include_directories(UI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(UI PUBLIC
    Core
    Rendering
    Physics
    ${NFD_LIBRARIES}
    glad::glad
    glfw
)
target_compile_definitions(UI PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)


# Platform-specific ImGui linking
if(WIN32)
    if(TARGET imgui::imgui)
        target_link_libraries(UI PUBLIC imgui::imgui)
    else()
        target_link_libraries(UI PUBLIC imgui)
    endif()
else()
    target_link_libraries(UI PUBLIC imgui)
endif()

target_compile_features(UI PUBLIC cxx_std_20)
