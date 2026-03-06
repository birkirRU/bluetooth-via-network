#pragma once

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <vector>
#include <cstdint>

constexpr int SAMPLE_RATE = 48000;
constexpr int CHANNELS = 2;
constexpr int FRAME_SIZE_MS = 20;
constexpr int FRAME_SIZE = SAMPLE_RATE * FRAME_SIZE_MS / 1000;
constexpr int BYTES_PER_SAMPLE = sizeof(float);
constexpr int PCM_BUFFER_SIZE = FRAME_SIZE * CHANNELS * BYTES_PER_SAMPLE;

class AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();

    bool Initialize();
    bool Start();
    void Stop();
    bool IsRunning() const { return _running; }

    bool GetCapturedData(std::vector<uint8_t>& buffer);

private:
    static HRESULT DeviceCallback(IAudioCaptureClient* pCaptureClient, UINT32 FramesAvailable);

    IMMDeviceEnumerator* _pEnumerator = nullptr;
    IMMDevice* _pDevice = nullptr;
    IAudioClient* _pAudioClient = nullptr;
    IAudioCaptureClient* _pCaptureClient = nullptr;
    HANDLE _hAudioAvailableEvent = nullptr;
    bool _running = false;
    std::vector<uint8_t> _pcmBuffer;
};
