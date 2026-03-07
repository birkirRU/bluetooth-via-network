#include <opus/opus.h>
#include <stdlib.h>
#include <stdio.h>

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_SIZE 960

typedef struct {
    OpusDecoder *decoder;
    int error;
} OpusDecoderCtx;

int audio_decoder_init(OpusDecoderCtx *ctx) {
    ctx->decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &ctx->error);
    return ctx->error;
}

void audio_decoder_destroy(OpusDecoderCtx *ctx) {
    if (ctx->decoder) {
        opus_decoder_destroy(ctx->decoder);
        ctx->decoder = NULL;
    }
}

int audio_decode_frame(OpusDecoderCtx *ctx, const uint8_t *opus_data, int opus_len, float *pcm_out) {
    if (!ctx->decoder) {
        return -1;
    }

    if (opus_data == NULL || opus_len == 0) {
        return opus_decode_float(ctx->decoder, NULL, 0, pcm_out, FRAME_SIZE, 1);
    }

    return opus_decode_float(ctx->decoder, opus_data, opus_len, pcm_out, FRAME_SIZE, 0);
}
