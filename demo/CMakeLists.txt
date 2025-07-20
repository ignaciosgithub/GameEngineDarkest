add_executable(GameEngineDemo
    main.cpp
)

target_include_directories(GameEngineDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(GameEngineDemo PRIVATE
    Core
    Rendering
    Physics
    # UI  # Temporarily disabled
)

target_compile_features(GameEngineDemo PRIVATE cxx_std_20)
