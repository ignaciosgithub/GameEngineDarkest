add_executable(GameEngineDemo
    main.cpp
)

add_executable(GameObjectDemo
    GameObjectDemo.cpp
)

target_include_directories(GameEngineDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_include_directories(GameObjectDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(GameEngineDemo PRIVATE
    Core
    Rendering
    Physics
    Physics2D
    # UI  # Temporarily disabled
)

target_link_libraries(GameObjectDemo PRIVATE
    Core
    Rendering
    Physics
    Physics2D
    # UI  # Temporarily disabled
)

target_compile_features(GameEngineDemo PRIVATE cxx_std_20)
target_compile_features(GameObjectDemo PRIVATE cxx_std_20)
