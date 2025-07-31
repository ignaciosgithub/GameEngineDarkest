add_library(Animation STATIC
    AnimationSystem.cpp
    Animator.cpp
)

target_include_directories(Animation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Animation PUBLIC
    Core
)

target_compile_features(Animation PUBLIC cxx_std_20)
