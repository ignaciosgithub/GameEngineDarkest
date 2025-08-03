add_library(Physics STATIC
    PhysicsWorld.cpp
    RigidBody/RigidBody.cpp
    Collision/CollisionDetection.cpp
    Collision/ContinuousCollisionDetection.cpp
    Spatial/Octree.cpp
    Colliders/ColliderShape.cpp
    Materials/PhysicsMaterial.cpp
    ../Core/Components/ColliderComponent.cpp
)

target_include_directories(Physics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Physics PUBLIC
    Core
)

target_compile_features(Physics PUBLIC cxx_std_20)

# Add 2D Physics subdirectory
add_subdirectory(2D)
