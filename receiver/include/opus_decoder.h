#ifndef OPUS_DECODER_H
#define OPUS_DECODER_H

#include <stdint.h>

typedef struct {
    void *decoder;
    int error;
} OpusDecoderCtx;

int audio_decoder_init(OpusDecoderCtx *ctx);
void audio_decoder_destroy(OpusDecoderCtx *ctx);
int audio_decode_frame(OpusDecoderCtx *ctx, const uint8_t *opus_data, int opus_len, float *pcm_out);

#endif
