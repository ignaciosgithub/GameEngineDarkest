#pragma once

#include "Socket.h"
#include <string>
#include <vector>
#include <memory>

namespace GameEngine {
    
    class TCPSocket : public Socket {
    public:
        TCPSocket();
        explicit TCPSocket(SocketHandle existingSocket);
        ~TCPSocket() override;
        
        // Socket interface implementation
        bool Initialize() override;
        
        // TCP-specific methods
        bool Listen(int backlog = 5);
        std::unique_ptr<TCPSocket> Accept();
        
        int Send(const std::vector<uint8_t>& data) override;
        int Receive(std::vector<uint8_t>& data, size_t maxSize = 1024) override;
        
        // Connection state
        bool IsConnected() const;
        void Disconnect();
        
        // TCP-specific options
        bool SetNoDelay(bool noDelay);
        bool SetKeepAlive(bool keepAlive);
        
        // Connection info
        std::string GetPeerAddress() const;
        int GetPeerPort() const;
        
    private:
        bool m_isConnected = false;
        bool m_isListening = false;
        std::string m_peerAddress;
        int m_peerPort = 0;
        
        void UpdateConnectionInfo();
    };
}
