#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "packet.h"
#include "jitter_buffer.h"

#define RECV_BUFFER_SIZE 4096

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
    JitterBuffer *jb;
    bool *running;
} UDPReceiver;

int udp_receiver_init(UDPReceiver *ur, int port, JitterBuffer *jb, bool *running) {
    ur->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ur->sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&ur->addr, 0, sizeof(ur->addr));
    ur->addr.sin_family = AF_INET;
    ur->addr.sin_addr.s_addr = INADDR_ANY;
    ur->addr.sin_port = htons(port);

    if (bind(ur->sockfd, (struct sockaddr *)&ur->addr, sizeof(ur->addr)) < 0) {
        perror("bind");
        close(ur->sockfd);
        return -1;
    }

    ur->jb = jb;
    ur->running = running;
    return 0;
}

void udp_receiver_destroy(UDPReceiver *ur) {
    if (ur->sockfd >= 0) {
        close(ur->sockfd);
        ur->sockfd = -1;
    }
}

void *udp_receiver_thread(void *arg) {
    UDPReceiver *ur = (UDPReceiver *)arg;
    uint8_t buffer[RECV_BUFFER_SIZE];
    AudioPacket packet;

    printf("UDP receiver listening on port %d\n", ntohs(ur->addr.sin_port));

    while (*ur->running) {
        ssize_t len = recvfrom(ur->sockfd, buffer, RECV_BUFFER_SIZE, 0, NULL, NULL);
        if (len < 0) {
            perror("recvfrom");
            continue;
        }

        if (len < sizeof(PacketHeader)) {
            continue;
        }

        memcpy(&packet.header, buffer, sizeof(PacketHeader));

        if (packet.header.payload_size > MAX_PAYLOAD_SIZE) {
            continue;
        }

        if (len - sizeof(PacketHeader) < packet.header.payload_size) {
            continue;
        }

        memcpy(packet.payload, buffer + sizeof(PacketHeader), packet.header.payload_size);

        jitter_buffer_push(ur->jb, &packet);
    }

    return NULL;
}
