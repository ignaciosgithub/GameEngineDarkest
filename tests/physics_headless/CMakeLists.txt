cmake_minimum_required(VERSION 3.16)

project(PhysicsHeadlessTest CXX)

add_executable(PhysicsHeadlessTest
    main.cpp
)

target_compile_features(PhysicsHeadlessTest PRIVATE cxx_std_20)

# Link against engine module targets (no rendering/UI)
target_link_libraries(PhysicsHeadlessTest PRIVATE
    Core
    Physics
    Physics2D
)

# Output to bin folder like the rest
set_target_properties(PhysicsHeadlessTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
