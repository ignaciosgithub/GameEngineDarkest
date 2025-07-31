add_library(Audio STATIC
    AudioManager.cpp
    AudioClip.cpp
    AudioSource.cpp
)

target_include_directories(Audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Audio PUBLIC
    Core
)

# Find and link OpenAL (optional)
if(WIN32)
    find_package(OpenAL CONFIG)
    if(OpenAL_FOUND)
        target_link_libraries(Audio PRIVATE OpenAL::OpenAL)
        target_compile_definitions(Audio PRIVATE OPENAL_AVAILABLE)
    endif()
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENAL openal)
    if(OPENAL_FOUND)
        target_link_libraries(Audio PRIVATE ${OPENAL_LIBRARIES})
        target_include_directories(Audio PRIVATE ${OPENAL_INCLUDE_DIRS})
        target_compile_definitions(Audio PRIVATE OPENAL_AVAILABLE)
    endif()
endif()

target_compile_features(Audio PUBLIC cxx_std_20)
