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
    /usr/include/imgui
)

target_link_libraries(UI PUBLIC
    Core
    Rendering
    ${NFD_LIBRARIES}
)

# Platform-specific ImGui linking
if(WIN32)
    if(TARGET imgui::imgui)
        target_link_libraries(UI PUBLIC imgui::imgui)
    else()
        target_link_libraries(UI PUBLIC imgui)
    endif()
else()
    # Use system-installed ImGui on Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(IMGUI REQUIRED imgui)
    target_include_directories(UI PUBLIC ${IMGUI_INCLUDE_DIRS})
    target_link_libraries(UI PUBLIC ${IMGUI_LIBRARIES})
endif()

target_compile_features(UI PUBLIC cxx_std_20)
