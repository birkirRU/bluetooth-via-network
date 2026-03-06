#include "audio_capture.h"
#include "opus_encoder.h"
#include "network_sender.h"

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <iostream>
#include <cstring>

std::atomic<bool> g_running(true);
std::atomic<bool> g_audioRunning(true);

std::queue<std::vector<uint8_t>> g_pcmQueue;
std::mutex g_pcmMutex;
std::condition_variable g_pcmCV;

std::queue<std::tuple<std::vector<uint8_t>, uint32_t, uint32_t>> g_packetQueue;
std::mutex g_packetMutex;
std::condition_variable g_packetCV;

void AudioCaptureThread(AudioCapture& capture)
{
    std::vector<uint8_t> pcmData;
    auto lastTime = std::chrono::steady_clock::now();

    while (g_running)
    {
        if (capture.GetCapturedData(pcmData))
        {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();
            lastTime = now;

            {
                std::lock_guard<std::mutex> lock(g_pcmMutex);
                g_pcmQueue.push(pcmData);
            }
            g_pcmCV.notify_one();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void EncoderThread(OpusEncoder& encoder, uint32_t& sequenceNumber)
{
    std::vector<uint8_t> pcmData;
    std::vector<uint8_t> opusData;
    uint32_t timestamp = 0;

    while (g_running)
    {
        {
            std::unique_lock<std::mutex> lock(g_pcmMutex);
            g_pcmCV.wait_for(lock, std::chrono::milliseconds(100), []
            { 
                return !g_pcmQueue.empty() || !g_running; 
            });

            if (!g_running && g_pcmQueue.empty())
                break;

            if (!g_pcmQueue.empty())
            {
                pcmData = std::move(g_pcmQueue.front());
                g_pcmQueue.pop();
            }
            else
            {
                continue;
            }
        }

        if (encoder.Encode(pcmData, opusData))
        {
            {
                std::lock_guard<std::mutex> lock(g_packetMutex);
                g_packetQueue.push(std::make_tuple(opusData, sequenceNumber++, timestamp));
            }
            g_packetCV.notify_one();
            timestamp += FRAME_SIZE;
        }
    }
}

void NetworkThread(NetworkSender& sender)
{
    std::vector<uint8_t> opusData;
    uint32_t seq = 0;
    uint32_t ts = 0;

    while (g_running)
    {
        {
            std::unique_lock<std::mutex> lock(g_packetMutex);
            g_packetCV.wait_for(lock, std::chrono::milliseconds(100), []
            { 
                return !g_packetQueue.empty() || !g_running; 
            });

            if (!g_running && g_packetQueue.empty())
                break;

            if (!g_packetQueue.empty())
            {
                auto [data, seqNum, tsNum] = std::move(g_packetQueue.front());
                g_packetQueue.pop();
                opusData = std::move(data);
                seq = seqNum;
                ts = tsNum;
            }
            else
            {
                continue;
            }
        }

        sender.Send(opusData, seq, ts);
    }
}

int main()
{
    std::cout << "Initializing Audio Streaming Client..." << std::endl;
    std::cout << "Sample Rate: " << SAMPLE_RATE << " Hz" << std::endl;
    std::cout << "Channels: " << CHANNELS << std::endl;
    std::cout << "Frame Size: " << FRAME_SIZE_MS << " ms" << std::endl;
    std::cout << "Server: " << SERVER_IP << ":" << SERVER_PORT << std::endl;

    AudioCapture capture;
    OpusEncoder encoder;
    NetworkSender sender;

    if (!capture.Initialize())
    {
        std::cerr << "Failed to initialize audio capture" << std::endl;
        return 1;
    }

    if (!encoder.Initialize())
    {
        std::cerr << "Failed to initialize Opus encoder" << std::endl;
        return 1;
    }

    if (!sender.Initialize(SERVER_IP, SERVER_PORT))
    {
        std::cerr << "Failed to initialize network sender" << std::endl;
        return 1;
    }

    if (!capture.Start())
    {
        std::cerr << "Failed to start audio capture" << std::endl;
        return 1;
    }

    uint32_t sequenceNumber = 0;

    std::thread captureThread(AudioCaptureThread, std::ref(capture));
    std::thread encoderThread(EncoderThread, std::ref(encoder), std::ref(sequenceNumber));
    std::thread networkThread(NetworkThread, std::ref(sender));

    std::cout << "Streaming started. Press Enter to stop..." << std::endl;
    std::cin.get();

    g_running = false;

    g_pcmCV.notify_all();
    g_packetCV.notify_all();

    captureThread.join();
    encoderThread.join();
    networkThread.join();

    capture.Stop();
    sender.Close();

    std::cout << "Streaming stopped." << std::endl;
    return 0;
}
