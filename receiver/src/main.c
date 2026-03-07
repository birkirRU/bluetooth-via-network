#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <opus/opus.h>

#include "packet.h"
#include "jitter_buffer.h"
#include "udp_receiver.h"
#include "opus_decoder.h"

#define DEFAULT_PORT 5000
#define PCM_FRAME_SIZE 960

static bool g_running = true;

void signal_handler(int sig) {
    (void)sig;
    g_running = false;
}

void *decoder_thread(void *arg) {
    JitterBuffer *jb = (JitterBuffer *)arg;
    OpusDecoderCtx decoder;
    float pcm_out[PCM_FRAME_SIZE * 2];
    AudioPacket packet;
    int decoded;
    size_t pcm_bytes;

    if (audio_decoder_init(&decoder) != OPUS_OK) {
        fprintf(stderr, "Failed to initialize Opus decoder\n");
        return NULL;
    }

    printf("Decoder thread started\n");

    while (g_running) {
        if (!jitter_buffer_pop(jb, &packet)) {
            continue;
        }

        decoded = audio_decode_frame(&decoder, packet.payload, packet.header.payload_size, pcm_out);

        if (decoded > 0) {
            pcm_bytes = decoded * 2 * sizeof(float);
            fwrite(pcm_out, 1, pcm_bytes, stdout);
        }
    }

    audio_decoder_destroy(&decoder);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    JitterBuffer jb;
    UDPReceiver ur;
    pthread_t recv_thread, dec_thread;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("Audio Receiver\n");
    printf("Port: %d\n", port);
    printf("Jitter buffer size: %d packets\n", JITTER_BUFFER_SIZE);

    jitter_buffer_init(&jb);

    if (udp_receiver_init(&ur, port, &jb, &g_running) < 0) {
        fprintf(stderr, "Failed to initialize UDP receiver\n");
        return 1;
    }

    pthread_create(&recv_thread, NULL, udp_receiver_thread, &ur);
    pthread_create(&dec_thread, NULL, decoder_thread, &jb);

    pthread_join(recv_thread, NULL);
    pthread_join(dec_thread, NULL);

    udp_receiver_destroy(&ur);
    jitter_buffer_destroy(&jb);

    printf("Receiver stopped\n");
    return 0;
}
