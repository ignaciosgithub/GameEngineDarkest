#include "Socket.h"
#include "../Core/Logging/Logger.h"
#include <cstring>

#ifdef _WIN32
    #pragma comment(lib, "ws2_32.lib")
#endif

namespace GameEngine {
    
    Socket::Socket(SocketType type) 
        : m_socket(INVALID_SOCKET_HANDLE), m_type(type), m_port(0), m_initialized(false) {
    }
    
    Socket::~Socket() {
        Close();
    }
    
    void Socket::Close() {
        if (m_socket != INVALID_SOCKET_HANDLE) {
#ifdef _WIN32
            closesocket(m_socket);
#else
            close(m_socket);
#endif
            m_socket = INVALID_SOCKET_HANDLE;
        }
        m_initialized = false;
    }
    
    bool Socket::IsValid() const {
        return m_socket != INVALID_SOCKET_HANDLE;
    }
    
    bool Socket::Bind(int port) {
        if (!IsValid()) {
            Logger::Error("Socket not initialized");
            return false;
        }
        
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(port));
        
        if (bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            Logger::Error("Failed to bind socket to port " + std::to_string(port) + ": " + GetLastErrorString());
            return false;
        }
        
        m_port = port;
        Logger::Debug("Socket bound to port " + std::to_string(port));
        return true;
    }
    
    bool Socket::Connect(const std::string& address, int port) {
        if (!IsValid()) {
            Logger::Error("Socket not initialized");
            return false;
        }
        
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(port));
        
#ifdef _WIN32
        if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
#else
        if (inet_aton(address.c_str(), &addr.sin_addr) == 0) {
#endif
            Logger::Error("Invalid address: " + address);
            return false;
        }
        
        if (connect(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            int error = GetLastError();
            if (!IsWouldBlock(error)) {
                Logger::Error("Failed to connect to " + address + ":" + std::to_string(port) + ": " + GetLastErrorString());
                return false;
            }
        }
        
        m_address = address;
        m_port = port;
        Logger::Debug("Connected to " + address + ":" + std::to_string(port));
        return true;
    }
    
    bool Socket::SetNonBlocking(bool nonBlocking) {
        if (!IsValid()) {
            Logger::Error("Socket not initialized");
            return false;
        }
        
#ifdef _WIN32
        u_long mode = nonBlocking ? 1 : 0;
        if (ioctlsocket(m_socket, FIONBIO, &mode) != 0) {
            Logger::Error("Failed to set non-blocking mode: " + GetLastErrorString());
            return false;
        }
#else
        int flags = fcntl(m_socket, F_GETFL, 0);
        if (flags < 0) {
            Logger::Error("Failed to get socket flags: " + GetLastErrorString());
            return false;
        }
        
        if (nonBlocking) {
            flags |= O_NONBLOCK;
        } else {
            flags &= ~O_NONBLOCK;
        }
        
        if (fcntl(m_socket, F_SETFL, flags) < 0) {
            Logger::Error("Failed to set non-blocking mode: " + GetLastErrorString());
            return false;
        }
#endif
        
        Logger::Debug("Socket non-blocking mode set to " + std::string(nonBlocking ? "true" : "false"));
        return true;
    }
    
    bool Socket::SetReuseAddress(bool reuse) {
        if (!IsValid()) {
            Logger::Error("Socket not initialized");
            return false;
        }
        
        int optval = reuse ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, 
                      reinterpret_cast<const char*>(&optval), sizeof(optval)) < 0) {
            Logger::Error("Failed to set reuse address: " + GetLastErrorString());
            return false;
        }
        
        Logger::Debug("Socket reuse address set to " + std::string(reuse ? "true" : "false"));
        return true;
    }
    
    bool Socket::InitializeNetworking() {
#ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            Logger::Error("WSAStartup failed: " + std::to_string(result));
            return false;
        }
        Logger::Info("Winsock initialized successfully");
#endif
        return true;
    }
    
    void Socket::CleanupNetworking() {
#ifdef _WIN32
        WSACleanup();
        Logger::Info("Winsock cleaned up");
#endif
    }
    
    std::string Socket::GetLastErrorString() {
#ifdef _WIN32
        int error = WSAGetLastError();
        char* message = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      nullptr, error, 0, reinterpret_cast<char*>(&message), 0, nullptr);
        std::string result = message ? message : "Unknown error";
        LocalFree(message);
        return result + " (Code: " + std::to_string(error) + ")";
#else
        return std::string(strerror(errno)) + " (Code: " + std::to_string(errno) + ")";
#endif
    }
    
    int Socket::GetLastError() const {
#ifdef _WIN32
        return WSAGetLastError();
#else
        return errno;
#endif
    }
    
    bool Socket::IsWouldBlock(int error) const {
#ifdef _WIN32
        return error == WSAEWOULDBLOCK;
#else
        return error == EWOULDBLOCK || error == EAGAIN;
#endif
    }
}
