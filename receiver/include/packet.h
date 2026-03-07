#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define MAX_PAYLOAD_SIZE 4000

#pragma pack(push, 1)
typedef struct {
    uint32_t sequence;
    uint32_t timestamp;
    uint16_t payload_size;
} PacketHeader;
#pragma pack(pop)

typedef struct {
    PacketHeader header;
    uint8_t payload[MAX_PAYLOAD_SIZE];
} AudioPacket;

#endif
