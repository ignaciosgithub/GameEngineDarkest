cmake_minimum_required(VERSION 3.16)
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
    # Probe vcpkg/include layout for ImGui backends or flat headers
    if(DEFINED VCPKG_TARGET_TRIPLET)
        set(_ui_imgui_roots "")
        if(DEFINED VCPKG_ROOT)
            list(APPEND _ui_imgui_roots "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include")
        endif()
        list(APPEND _ui_imgui_roots "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include")
        get_filename_component(_repo_root "${CMAKE_SOURCE_DIR}" DIRECTORY)
        list(APPEND _ui_imgui_roots "${_repo_root}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include")
        set(_ui_imgui_inc_found FALSE)
        foreach(_root IN LISTS _ui_imgui_roots)
            if(EXISTS "${_root}/imgui/backends/imgui_impl_glfw.h")
                message(STATUS "[UI] Using ImGui backends include: ${_root}/imgui/backends")
                target_include_directories(UI PUBLIC "${_root}/imgui/backends")
                set(_ui_imgui_inc_found TRUE)
                break()
            elseif(EXISTS "${_root}/backends/imgui_impl_glfw.h")
                message(STATUS "[UI] Using ImGui backends include: ${_root}/backends")
                target_include_directories(UI PUBLIC "${_root}/backends")
                set(_ui_imgui_inc_found TRUE)
                break()
            elseif(EXISTS "${_root}/imgui_impl_glfw.h")
                message(STATUS "[UI] Using flat ImGui include: ${_root}")
                target_include_directories(UI PUBLIC "${_root}")
                set(_ui_imgui_inc_found TRUE)
                break()
            endif()
        endforeach()
        if(NOT _ui_imgui_inc_found)
            message(STATUS "[UI] ImGui backends include not auto-detected; relying on imported targets/backends fallbacks")
        endif()
    endif()
        endif()
    endif()
else()
    target_link_libraries(UI PUBLIC imgui)
endif()

target_compile_features(UI PUBLIC cxx_std_20)
