#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <stdbool.h>
#include "jitter_buffer.h"

typedef struct {
    int sockfd;
    void *addr;
    JitterBuffer *jb;
    bool *running;
} UDPReceiver;

int udp_receiver_init(UDPReceiver *ur, int port, JitterBuffer *jb, bool *running);
void udp_receiver_destroy(UDPReceiver *ur);
void *udp_receiver_thread(void *arg);

#endif
