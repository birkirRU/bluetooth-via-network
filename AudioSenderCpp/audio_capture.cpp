#include "audio_capture.h"
#include <combaseapi.h>
#include <cstring>
#include <thread>
#include <atomic>

extern std::atomic<bool> g_audioRunning;

AudioCapture::AudioCapture() : _pcmBuffer(PCM_BUFFER_SIZE)
{
}

AudioCapture::~AudioCapture()
{
    Stop();
}

bool AudioCapture::Initialize()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        return false;
    }

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&_pEnumerator
    );
    if (FAILED(hr)) return false;

    hr = _pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &_pDevice);
    if (FAILED(hr)) return false;

    hr = _pDevice->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        (void**)&_pAudioClient
    );
    if (FAILED(hr)) return false;

    WAVEFORMATEX* pWaveFormat = nullptr;
    hr = _pAudioClient->GetMixFormat(&pWaveFormat);
    if (FAILED(hr)) return false;

    pWaveFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    pWaveFormat->nChannels = CHANNELS;
    pWaveFormat->nSamplesPerSec = SAMPLE_RATE;
    pWaveFormat->wBitsPerSample = 32;
    pWaveFormat->nBlockAlign = pWaveFormat->nChannels * (pWaveFormat->wBitsPerSample / 8);
    pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign;

    hr = _pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        10000000,
        0,
        pWaveFormat,
        nullptr
    );
    CoTaskMemFree(pWaveFormat);
    if (FAILED(hr)) return false;

    hr = _pAudioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&_pCaptureClient
    );
    if (FAILED(hr)) return false;

    _hAudioAvailableEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!_hAudioAvailableEvent) return false;

    hr = _pAudioClient->SetEventHandle(_hAudioAvailableEvent);
    if (FAILED(hr)) return false;

    return true;
}

bool AudioCapture::Start()
{
    if (!_pAudioClient || !_pCaptureClient) return false;

    HRESULT hr = _pAudioClient->Start();
    if (FAILED(hr)) return false;

    _running = true;
    return true;
}

void AudioCapture::Stop()
{
    _running = false;
    if (_pAudioClient)
    {
        _pAudioClient->Stop();
    }
    if (_hAudioAvailableEvent)
    {
        CloseHandle(_hAudioAvailableEvent);
        _hAudioAvailableEvent = nullptr;
    }
}

bool AudioCapture::GetCapturedData(std::vector<uint8_t>& buffer)
{
    if (!_running || !_pCaptureClient)
    {
        return false;
    }

    UINT32 framesAvailable = 0;
    HRESULT hr = _pCaptureClient->GetNextPacketSize(&framesAvailable);
    if (FAILED(hr) || framesAvailable == 0)
    {
        return false;
    }

    BYTE* pData = nullptr;
    UINT32 framesToRead = framesAvailable;
    DWORD flags = 0;

    hr = _pCaptureClient->GetBuffer(&pData, &framesToRead, &flags, nullptr, nullptr);
    if (FAILED(hr))
    {
        return false;
    }

    UINT32 bytesToCopy = framesToRead * CHANNELS * sizeof(float);
    if (bytesToCopy <= PCM_BUFFER_SIZE)
    {
        std::memcpy(_pcmBuffer.data(), pData, bytesToCopy);
        buffer.assign(_pcmBuffer.begin(), _pcmBuffer.begin() + bytesToCopy);
    }

    hr = _pCaptureClient->ReleaseBuffer(framesToRead);
    return SUCCEEDED(hr) && !buffer.empty();
}
