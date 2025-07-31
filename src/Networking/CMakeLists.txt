add_library(Networking STATIC
    NetworkManager.cpp
    Socket.cpp
    UDPSocket.cpp
    TCPSocket.cpp
)

target_include_directories(Networking PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Networking PUBLIC
    Core
)

# Platform-specific networking libraries
if(WIN32)
    target_link_libraries(Networking PRIVATE ws2_32)
endif()

target_compile_features(Networking PUBLIC cxx_std_20)
