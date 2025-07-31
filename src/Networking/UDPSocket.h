#pragma once

#include "Socket.h"
#include <string>
#include <vector>

namespace GameEngine {
    
    class UDPSocket : public Socket {
    public:
        UDPSocket();
        ~UDPSocket() override;
        
        // Socket interface implementation
        bool Initialize() override;
        
        // UDP-specific methods
        int Send(const std::vector<uint8_t>& data) override;
        int Send(const std::vector<uint8_t>& data, const std::string& address, int port);
        int Receive(std::vector<uint8_t>& data, size_t maxSize = 1024) override;
        int ReceiveFrom(std::vector<uint8_t>& data, std::string& senderAddress, int& senderPort, size_t maxSize = 1024);
        
        // UDP broadcast support
        bool EnableBroadcast(bool enable);
        int Broadcast(const std::vector<uint8_t>& data, int port);
        
        // Multicast support
        bool JoinMulticastGroup(const std::string& multicastAddress);
        bool LeaveMulticastGroup(const std::string& multicastAddress);
        
    private:
        std::string m_defaultTargetAddress;
        int m_defaultTargetPort = 0;
        bool m_broadcastEnabled = false;
    };
}
