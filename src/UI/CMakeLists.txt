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
)
if(TARGET glad::glad)
    target_link_libraries(UI PUBLIC glad::glad)
endif()
if(TARGET glfw)
    target_link_libraries(UI PUBLIC glfw)
endif()
target_compile_definitions(UI PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)


# Platform-specific ImGui linking
if(WIN32)
    if(TARGET imgui::imgui)
        target_link_libraries(UI PUBLIC imgui::imgui)
        if(TARGET imgui::imgui_glfw)
            target_link_libraries(UI PUBLIC imgui::imgui_glfw)
        endif()
        if(TARGET imgui::imgui_opengl3)
            target_link_libraries(UI PUBLIC imgui::imgui_opengl3)
        endif()
    else()
        target_link_libraries(UI PUBLIC imgui)
    endif()
else()
# Fallback include for ImGui backends on Windows if imported targets don't expose backends include dir
if(WIN32 AND TARGET imgui::imgui AND NOT TARGET imgui::imgui_glfw)
    get_target_property(_imgui_inc imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)
    if(_imgui_inc)
        target_include_directories(UI PUBLIC "${_imgui_inc}/backends")
    endif()
endif()

    target_link_libraries(UI PUBLIC imgui)
endif()

target_compile_features(UI PUBLIC cxx_std_20)
