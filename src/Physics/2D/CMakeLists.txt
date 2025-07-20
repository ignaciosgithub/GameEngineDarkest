add_library(Physics2D STATIC
    RigidBody2D.cpp
    Colliders/Collider2D.cpp
    Colliders/CircleCollider2D.cpp
    Colliders/BoxCollider2D.cpp
    Collision/CollisionDetection2D.cpp
    Spatial/QuadTree.cpp
    PhysicsWorld2D.cpp
    Demo/Physics2DDemo.cpp
)

target_include_directories(Physics2D PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(Physics2D PUBLIC
    Core
)

target_compile_features(Physics2D PUBLIC cxx_std_20)
