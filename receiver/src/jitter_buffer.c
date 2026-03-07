#include "jitter_buffer.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

void jitter_buffer_init(JitterBuffer *jb) {
    memset(jb, 0, sizeof(JitterBuffer));
    pthread_mutex_init(&jb->mutex, NULL);
    pthread_cond_init(&jb->cond, NULL);
    jb->next_sequence = 0;
    jb->running = true;
}

void jitter_buffer_destroy(JitterBuffer *jb) {
    jb->running = false;
    pthread_cond_broadcast(&jb->cond);
    pthread_mutex_destroy(&jb->mutex);
    pthread_cond_destroy(&jb->cond);
}

void jitter_buffer_set_next_sequence(JitterBuffer *jb, uint32_t seq) {
    pthread_mutex_lock(&jb->mutex);
    jb->next_sequence = seq;
    pthread_mutex_unlock(&jb->mutex);
}

void jitter_buffer_push(JitterBuffer *jb, const AudioPacket *packet) {
    pthread_mutex_lock(&jb->mutex);

    if (!jb->running) {
        pthread_mutex_unlock(&jb->mutex);
        return;
    }

    uint32_t seq = packet->header.sequence;
    int diff = (int)(seq - jb->next_sequence);

    if (diff < 0) {
        pthread_mutex_unlock(&jb->mutex);
        return;
    }

    if (diff >= JITTER_BUFFER_SIZE) {
        jb->next_sequence = seq - (JITTER_BUFFER_SIZE - 1);
        diff = JITTER_BUFFER_SIZE - 1;
    }

    int index = diff;
    jb->packets[index] = *packet;
    jb->valid[index] = true;

    pthread_cond_signal(&jb->cond);
    pthread_mutex_unlock(&jb->mutex);
}

bool jitter_buffer_pop(JitterBuffer *jb, AudioPacket *packet) {
    pthread_mutex_lock(&jb->mutex);

    while (jb->running && !jb->valid[0]) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        if (pthread_cond_timedwait(&jb->cond, &jb->mutex, &ts) == ETIMEDOUT) {
            pthread_mutex_unlock(&jb->mutex);
            return false;
        }
    }

    if (!jb->running) {
        pthread_mutex_unlock(&jb->mutex);
        return false;
    }

    *packet = jb->packets[0];
    jb->valid[0] = false;

    for (int i = 0; i < JITTER_BUFFER_SIZE - 1; i++) {
        jb->packets[i] = jb->packets[i + 1];
        jb->valid[i] = jb->valid[i + 1];
    }
    jb->valid[JITTER_BUFFER_SIZE - 1] = false;
    jb->next_sequence++;

    pthread_mutex_unlock(&jb->mutex);
    return true;
}
