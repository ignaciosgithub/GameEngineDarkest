cmake_minimum_required(VERSION 3.20)

project(project_io_demo)

add_executable(project_io_demo
    main.cpp
)

target_include_directories(project_io_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(project_io_demo PRIVATE Core Rendering)

set_target_properties(project_io_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
