#include "UDPSocket.h"
#include "../Core/Logging/Logger.h"
#include <cstring>

namespace GameEngine {
    
    UDPSocket::UDPSocket() : Socket(SocketType::UDP) {
    }
    
    UDPSocket::~UDPSocket() = default;
    
    bool UDPSocket::Initialize() {
        if (m_initialized) {
            Logger::Warning("UDPSocket already initialized");
            return true;
        }
        
        m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_socket == INVALID_SOCKET_HANDLE) {
            Logger::Error("Failed to create UDP socket: " + GetLastErrorString());
            return false;
        }
        
        SetReuseAddress(true);
        SetNonBlocking(true);
        
        m_initialized = true;
        Logger::Debug("UDP socket initialized successfully");
        return true;
    }
    
    int UDPSocket::Send(const std::vector<uint8_t>& data) {
        if (m_defaultTargetAddress.empty() || m_defaultTargetPort == 0) {
            Logger::Error("No default target address set for UDP socket");
            return -1;
        }
        
        return Send(data, m_defaultTargetAddress, m_defaultTargetPort);
    }
    
    int UDPSocket::Send(const std::vector<uint8_t>& data, const std::string& address, int port) {
        if (!IsValid()) {
            Logger::Error("UDP socket not initialized");
            return -1;
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
            return -1;
        }
        
        int bytesSent = sendto(m_socket, reinterpret_cast<const char*>(data.data()), 
                              static_cast<int>(data.size()), 0,
                              reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        
        if (bytesSent < 0) {
            int error = GetLastError();
            if (!IsWouldBlock(error)) {
                Logger::Error("Failed to send UDP data: " + GetLastErrorString());
            }
            return -1;
        }
        
        Logger::Debug("Sent " + std::to_string(bytesSent) + " bytes via UDP to " + 
                     address + ":" + std::to_string(port));
        return bytesSent;
    }
    
    int UDPSocket::Receive(std::vector<uint8_t>& data, size_t maxSize) {
        std::string senderAddress;
        int senderPort;
        return ReceiveFrom(data, senderAddress, senderPort, maxSize);
    }
    
    int UDPSocket::ReceiveFrom(std::vector<uint8_t>& data, std::string& senderAddress, 
                              int& senderPort, size_t maxSize) {
        if (!IsValid()) {
            Logger::Error("UDP socket not initialized");
            return -1;
        }
        
        data.resize(maxSize);
        sockaddr_in senderAddr{};
        socklen_t senderAddrLen = sizeof(senderAddr);
        
        int bytesReceived = recvfrom(m_socket, reinterpret_cast<char*>(data.data()), 
                                    static_cast<int>(maxSize), 0,
                                    reinterpret_cast<sockaddr*>(&senderAddr), &senderAddrLen);
        
        if (bytesReceived < 0) {
            int error = GetLastError();
            if (!IsWouldBlock(error)) {
                Logger::Error("Failed to receive UDP data: " + GetLastErrorString());
            }
            return -1;
        }
        
        data.resize(bytesReceived);
        
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &senderAddr.sin_addr, addressBuffer, INET_ADDRSTRLEN);
        senderAddress = addressBuffer;
        senderPort = ntohs(senderAddr.sin_port);
        
        Logger::Debug("Received " + std::to_string(bytesReceived) + " bytes via UDP from " + 
                     senderAddress + ":" + std::to_string(senderPort));
        return bytesReceived;
    }
    
    bool UDPSocket::EnableBroadcast(bool enable) {
        if (!IsValid()) {
            Logger::Error("UDP socket not initialized");
            return false;
        }
        
        int optval = enable ? 1 : 0;
        if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, 
                      reinterpret_cast<const char*>(&optval), sizeof(optval)) < 0) {
            Logger::Error("Failed to set broadcast option: " + GetLastErrorString());
            return false;
        }
        
        m_broadcastEnabled = enable;
        Logger::Debug("UDP broadcast " + std::string(enable ? "enabled" : "disabled"));
        return true;
    }
    
    int UDPSocket::Broadcast(const std::vector<uint8_t>& data, int port) {
        if (!m_broadcastEnabled) {
            Logger::Error("Broadcast not enabled on UDP socket");
            return -1;
        }
        
        return Send(data, "255.255.255.255", port);
    }
    
    bool UDPSocket::JoinMulticastGroup(const std::string& multicastAddress) {
        if (!IsValid()) {
            Logger::Error("UDP socket not initialized");
            return false;
        }
        
        ip_mreq mreq{};
#ifdef _WIN32
        if (inet_pton(AF_INET, multicastAddress.c_str(), &mreq.imr_multiaddr) <= 0) {
#else
        if (inet_aton(multicastAddress.c_str(), &mreq.imr_multiaddr) == 0) {
#endif
            Logger::Error("Invalid multicast address: " + multicastAddress);
            return false;
        }
        
        mreq.imr_interface.s_addr = INADDR_ANY;
        
        if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
                      reinterpret_cast<const char*>(&mreq), sizeof(mreq)) < 0) {
            Logger::Error("Failed to join multicast group: " + GetLastErrorString());
            return false;
        }
        
        Logger::Debug("Joined multicast group: " + multicastAddress);
        return true;
    }
    
    bool UDPSocket::LeaveMulticastGroup(const std::string& multicastAddress) {
        if (!IsValid()) {
            Logger::Error("UDP socket not initialized");
            return false;
        }
        
        ip_mreq mreq{};
#ifdef _WIN32
        if (inet_pton(AF_INET, multicastAddress.c_str(), &mreq.imr_multiaddr) <= 0) {
#else
        if (inet_aton(multicastAddress.c_str(), &mreq.imr_multiaddr) == 0) {
#endif
            Logger::Error("Invalid multicast address: " + multicastAddress);
            return false;
        }
        
        mreq.imr_interface.s_addr = INADDR_ANY;
        
        if (setsockopt(m_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, 
                      reinterpret_cast<const char*>(&mreq), sizeof(mreq)) < 0) {
            Logger::Error("Failed to leave multicast group: " + GetLastErrorString());
            return false;
        }
        
        Logger::Debug("Left multicast group: " + multicastAddress);
        return true;
    }
}
