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
    # OpenAL will be added here when implemented
    # ${OPENAL_LIBRARY}
)

target_compile_features(Audio PUBLIC cxx_std_20)
