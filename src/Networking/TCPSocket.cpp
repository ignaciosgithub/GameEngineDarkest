#include "TCPSocket.h"
#include "../Core/Logging/Logger.h"
#include <cstring>

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

namespace GameEngine {
    
    TCPSocket::TCPSocket() : Socket(SocketType::TCP) {
    }
    
    TCPSocket::TCPSocket(SocketHandle existingSocket) : Socket(SocketType::TCP) {
        m_socket = existingSocket;
        m_initialized = true;
        m_isConnected = true;
        UpdateConnectionInfo();
    }
    
    TCPSocket::~TCPSocket() {
        Disconnect();
    }
    
    bool TCPSocket::Initialize() {
        if (m_initialized) {
            Logger::Warning("TCPSocket already initialized");
            return true;
        }
        
        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET_HANDLE) {
            Logger::Error("Failed to create TCP socket: " + GetLastErrorString());
            return false;
        }
        
        SetReuseAddress(true);
        SetNonBlocking(true);
        
        m_initialized = true;
        Logger::Debug("TCP socket initialized successfully");
        return true;
    }
    
    bool TCPSocket::Listen(int backlog) {
        if (!IsValid()) {
            Logger::Error("TCP socket not initialized");
            return false;
        }
        
        if (listen(m_socket, backlog) < 0) {
            Logger::Error("Failed to listen on TCP socket: " + GetLastErrorString());
            return false;
        }
        
        m_isListening = true;
        Logger::Debug("TCP socket listening with backlog " + std::to_string(backlog));
        return true;
    }
    
    std::unique_ptr<TCPSocket> TCPSocket::Accept() {
        if (!IsValid() || !m_isListening) {
            Logger::Error("TCP socket not listening");
            return nullptr;
        }
        
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        SocketHandle clientSocket = accept(m_socket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
        if (clientSocket == INVALID_SOCKET_HANDLE) {
            int error = GetLastError();
            if (!IsWouldBlock(error)) {
                Logger::Error("Failed to accept connection: " + GetLastErrorString());
            }
            return nullptr;
        }
        
        auto client = std::make_unique<TCPSocket>(clientSocket);
        
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, addressBuffer, INET_ADDRSTRLEN);
        client->m_peerAddress = addressBuffer;
        client->m_peerPort = ntohs(clientAddr.sin_port);
        
        Logger::Debug("Accepted TCP connection from " + client->m_peerAddress + ":" + 
                     std::to_string(client->m_peerPort));
        return client;
    }
    
    int TCPSocket::Send(const std::vector<uint8_t>& data) {
        if (!IsValid() || !m_isConnected) {
            Logger::Error("TCP socket not connected");
            return -1;
        }
        
        int bytesSent = send(m_socket, reinterpret_cast<const char*>(data.data()), 
                            static_cast<int>(data.size()), 0);
        
        if (bytesSent < 0) {
            int error = GetLastError();
            if (!IsWouldBlock(error)) {
                Logger::Error("Failed to send TCP data: " + GetLastErrorString());
                m_isConnected = false;
            }
            return -1;
        }
        
        Logger::Debug("Sent " + std::to_string(bytesSent) + " bytes via TCP");
        return bytesSent;
    }
    
    int TCPSocket::Receive(std::vector<uint8_t>& data, size_t maxSize) {
        if (!IsValid() || !m_isConnected) {
            Logger::Error("TCP socket not connected");
            return -1;
        }
        
        data.resize(maxSize);
        int bytesReceived = recv(m_socket, reinterpret_cast<char*>(data.data()), 
                                static_cast<int>(maxSize), 0);
        
        if (bytesReceived < 0) {
            int error = GetLastError();
            if (!IsWouldBlock(error)) {
                Logger::Error("Failed to receive TCP data: " + GetLastErrorString());
                m_isConnected = false;
            }
            return -1;
        } else if (bytesReceived == 0) {
            Logger::Debug("TCP connection closed by peer");
            m_isConnected = false;
            return 0;
        }
        
        data.resize(bytesReceived);
        Logger::Debug("Received " + std::to_string(bytesReceived) + " bytes via TCP");
        return bytesReceived;
    }
    
    bool TCPSocket::IsConnected() const {
        return m_isConnected && IsValid();
    }
    
    void TCPSocket::Disconnect() {
        if (IsValid()) {
#ifdef _WIN32
            shutdown(m_socket, SD_BOTH);
#else
            shutdown(m_socket, SHUT_RDWR);
#endif
        }
        
        m_isConnected = false;
        m_isListening = false;
        Close();
        
        Logger::Debug("TCP socket disconnected");
    }
    
    bool TCPSocket::SetNoDelay(bool noDelay) {
        if (!IsValid()) {
            Logger::Error("TCP socket not initialized");
            return false;
        }
        
        int optval = noDelay ? 1 : 0;
        if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, 
                      reinterpret_cast<const char*>(&optval), sizeof(optval)) < 0) {
            Logger::Error("Failed to set TCP_NODELAY: " + GetLastErrorString());
            return false;
        }
        
        Logger::Debug("TCP_NODELAY set to " + std::string(noDelay ? "true" : "false"));
        return true;
    }
    
    bool TCPSocket::SetKeepAlive(bool keepAlive) {
        if (!IsValid()) {
            Logger::Error("TCP socket not initialized");
            return false;
        }
        
        int optval = keepAlive ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, 
                      reinterpret_cast<const char*>(&optval), sizeof(optval)) < 0) {
            Logger::Error("Failed to set SO_KEEPALIVE: " + GetLastErrorString());
            return false;
        }
        
        Logger::Debug("SO_KEEPALIVE set to " + std::string(keepAlive ? "true" : "false"));
        return true;
    }
    
    std::string TCPSocket::GetPeerAddress() const {
        return m_peerAddress;
    }
    
    int TCPSocket::GetPeerPort() const {
        return m_peerPort;
    }
    
    void TCPSocket::UpdateConnectionInfo() {
        if (!IsValid()) return;
        
        sockaddr_in addr{};
        socklen_t addrLen = sizeof(addr);
        
        if (getpeername(m_socket, reinterpret_cast<sockaddr*>(&addr), &addrLen) == 0) {
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr.sin_addr, addressBuffer, INET_ADDRSTRLEN);
            m_peerAddress = addressBuffer;
            m_peerPort = ntohs(addr.sin_port);
        }
    }
}
