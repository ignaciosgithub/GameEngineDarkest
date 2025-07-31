#pragma once

#include <string>
#include <vector>
#include <cstdint>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET SocketHandle;
    #define INVALID_SOCKET_HANDLE INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int SocketHandle;
    #define INVALID_SOCKET_HANDLE -1
#endif

namespace GameEngine {
    
    enum class SocketType {
        UDP,
        TCP
    };
    
    class Socket {
    public:
        Socket(SocketType type);
        virtual ~Socket();
        
        // Basic socket operations
        virtual bool Initialize() = 0;
        virtual void Close();
        virtual bool IsValid() const;
        
        // Binding and connection
        virtual bool Bind(int port);
        virtual bool Connect(const std::string& address, int port);
        
        // Data transmission
        virtual int Send(const std::vector<uint8_t>& data) = 0;
        virtual int Receive(std::vector<uint8_t>& data, size_t maxSize = 1024) = 0;
        
        // Socket configuration
        bool SetNonBlocking(bool nonBlocking);
        bool SetReuseAddress(bool reuse);
        
        // Getters
        SocketHandle GetHandle() const { return m_socket; }
        SocketType GetType() const { return m_type; }
        int GetPort() const { return m_port; }
        const std::string& GetAddress() const { return m_address; }
        
        // Static utility methods
        static bool InitializeNetworking();
        static void CleanupNetworking();
        static std::string GetLastErrorString();
        
    protected:
        SocketHandle m_socket;
        SocketType m_type;
        std::string m_address;
        int m_port;
        bool m_initialized;
        
        // Platform-specific error handling
        int GetLastError() const;
        bool IsWouldBlock(int error) const;
    };
}
