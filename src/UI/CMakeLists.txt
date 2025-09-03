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
        if(NOT TARGET imgui::imgui_glfw OR NOT TARGET imgui::imgui_opengl3)
            get_target_property(_imgui_inc imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)
            if(_imgui_inc)
                target_include_directories(UI PUBLIC "${_imgui_inc}/backends")
            endif()
            if(DEFINED VCPKG_TARGET_TRIPLET)
                target_include_directories(UI PUBLIC
                    "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/imgui/backends"
                )
                if(DEFINED VCPKG_ROOT)
                    target_include_directories(UI PUBLIC
                        "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/imgui/backends"
                    )
                else()
                    get_filename_component(_repo_root "${CMAKE_SOURCE_DIR}" DIRECTORY)
                    target_include_directories(UI PUBLIC
                        "${_repo_root}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include/imgui/backends"
                    )
                endif()
            endif()
        endif()
    else()
        target_link_libraries(UI PUBLIC imgui)
        get_target_property(_imgui_inc imgui INTERFACE_INCLUDE_DIRECTORIES)
        if(_imgui_inc)
            target_include_directories(UI PUBLIC "${_imgui_inc}/backends")
        endif()
        if(DEFINED VCPKG_TARGET_TRIPLET)
            target_include_directories(UI PUBLIC
                "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include/imgui/backends"
            )
            if(DEFINED VCPKG_ROOT)
                target_include_directories(UI PUBLIC
                    "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/imgui/backends"
                )
            else()
                get_filename_component(_repo_root "${CMAKE_SOURCE_DIR}" DIRECTORY)
                target_include_directories(UI PUBLIC
                    "${_repo_root}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include/imgui/backends"
                )
            endif()
        endif()
    endif()
else()
    target_link_libraries(UI PUBLIC imgui)
endif()

target_compile_features(UI PUBLIC cxx_std_20)
