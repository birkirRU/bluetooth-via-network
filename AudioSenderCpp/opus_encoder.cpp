#include "opus_encoder.h"
#include <cstring>

OpusEncoder::OpusEncoder()
{
}

OpusEncoder::~OpusEncoder()
{
    if (_encoder)
    {
        opus_encoder_destroy(_encoder);
        _encoder = nullptr;
    }
}

bool OpusEncoder::Initialize(int sampleRate, int channels)
{
    _channels = channels;

    int error = 0;
    _encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &error);
    if (error != OPUS_OK || !_encoder)
    {
        return false;
    }

    opus_encoder_ctl(_encoder, OPUS_SET_BITRATE(64000));
    opus_encoder_ctl(_encoder, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(_encoder, OPUS_SET_VBR(1));
    opus_encoder_ctl(_encoder, OPUS_SIGNAL(VOICE));

    return true;
}

bool OpusEncoder::Encode(const std::vector<uint8_t>& pcmData, std::vector<uint8_t>& opusData)
{
    if (!_encoder || pcmData.empty())
    {
        return false;
    }

    opusData.resize(MAX_PACKET_SIZE);

    const float* pcm = reinterpret_cast<const float*>(pcmData.data());
    int frameSize = static_cast<int>(pcmData.size() / (sizeof(float) * _channels));

    int outputSize = opus_encode_float(
        _encoder,
        pcm,
        frameSize,
        opusData.data(),
        MAX_PACKET_SIZE
    );

    if (outputSize < 0)
    {
        return false;
    }

    opusData.resize(outputSize);
    return true;
}
