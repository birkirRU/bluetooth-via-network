#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <cstdint>
#include <string>

#pragma comment(lib, "ws2_32.lib")

constexpr int SERVER_PORT = 5000;
constexpr const char* SERVER_IP = "192.168.1.100";

struct PacketHeader
{
    uint32_t sequence_number;
    uint32_t timestamp;
    uint16_t payload_size;
};

class NetworkSender
{
public:
    NetworkSender();
    ~NetworkSender();

    bool Initialize(const char* serverIp, int serverPort);
    bool Send(const std::vector<uint8_t>& opusData, uint32_t sequenceNumber, uint32_t timestamp);
    void Close();

private:
    SOCKET _sock = INVALID_SOCKET;
    sockaddr_in _serverAddr;
};
