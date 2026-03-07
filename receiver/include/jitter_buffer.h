#ifndef JITTER_BUFFER_H
#define JITTER_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "packet.h"

#define JITTER_BUFFER_SIZE 5

typedef struct {
    AudioPacket packets[JITTER_BUFFER_SIZE];
    bool valid[JITTER_BUFFER_SIZE];
    uint32_t next_sequence;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool running;
} JitterBuffer;

void jitter_buffer_init(JitterBuffer *jb);
void jitter_buffer_destroy(JitterBuffer *jb);
void jitter_buffer_push(JitterBuffer *jb, const AudioPacket *packet);
bool jitter_buffer_pop(JitterBuffer *jb, AudioPacket *packet);
void jitter_buffer_set_next_sequence(JitterBuffer *jb, uint32_t seq);

#endif
