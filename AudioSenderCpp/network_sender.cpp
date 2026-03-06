#include "network_sender.h"
#include <cstring>

NetworkSender::NetworkSender()
{
    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
}

NetworkSender::~NetworkSender()
{
    Close();
}

bool NetworkSender::Initialize(const char* serverIp, int serverPort)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        return false;
    }

    _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sock == INVALID_SOCKET)
    {
        WSACleanup();
        return false;
    }

    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp, &_serverAddr.sin_addr);

    return true;
}

bool NetworkSender::Send(const std::vector<uint8_t>& opusData, uint32_t sequenceNumber, uint32_t timestamp)
{
    if (_sock == INVALID_SOCKET || opusData.empty())
    {
        return false;
    }

    std::vector<uint8_t> packet(sizeof(PacketHeader) + opusData.size());
    
    PacketHeader header;
    header.sequence_number = sequenceNumber;
    header.timestamp = timestamp;
    header.payload_size = static_cast<uint16_t>(opusData.size());

    std::memcpy(packet.data(), &header, sizeof(PacketHeader));
    std::memcpy(packet.data() + sizeof(PacketHeader), opusData.data(), opusData.size());

    int sendResult = sendto(
        _sock,
        reinterpret_cast<const char*>(packet.data()),
        static_cast<int>(packet.size()),
        0,
        (sockaddr*)&_serverAddr,
        sizeof(_serverAddr)
    );

    return sendResult != SOCKET_ERROR;
}

void NetworkSender::Close()
{
    if (_sock != INVALID_SOCKET)
    {
        closesocket(_sock);
        _sock = INVALID_SOCKET;
    }
    WSACleanup();
}
