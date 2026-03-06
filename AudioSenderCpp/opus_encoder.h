#pragma once

#include <opus.h>
#include <vector>
#include <cstdint>

constexpr int MAX_PACKET_SIZE = 4000;

class OpusEncoder
{
public:
    OpusEncoder();
    ~OpusEncoder();

    bool Initialize(int sampleRate = SAMPLE_RATE, int channels = CHANNELS);
    bool Encode(const std::vector<uint8_t>& pcmData, std::vector<uint8_t>& opusData);

private:
    OpusEncoder* _encoder = nullptr;
    int _channels = CHANNELS;
};
