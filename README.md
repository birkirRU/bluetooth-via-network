# bluetooth-via-network

What you want is called **loopback audio capture**: recording the sound that the computer itself is playing (music, videos, games, etc.) rather than microphone input. On Windows this is done using **Windows Audio Session API (WASAPI)** in **loopback mode**.

This is actually how tools like **OBS Studio** capture “Desktop Audio”.

Below is the correct architecture and a clean implementation approach.

---

# 1. Core Idea

Instead of recording a microphone device, you capture the **output audio endpoint** and enable **loopback**.

Pipeline:

```
Windows Audio Output
      ↓
WASAPI Loopback Capture
      ↓
PCM audio buffer
      ↓
(optional) audio compression
      ↓
network socket
      ↓
server
```

---

# 2. Enabling Loopback Capture

The key difference from microphone capture is this flag when initializing the audio client:

```
AUDCLNT_STREAMFLAGS_LOOPBACK
```

This tells WASAPI:

> “Capture what the system is playing to speakers.”

---

# 3. Basic C++ Loopback Capture Example

Simplified example capturing system audio.

```cpp
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>

#pragma comment(lib, "ole32.lib")

int main() {
    CoInitialize(NULL);

    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDevice* device = nullptr;
    IAudioClient* audioClient = nullptr;
    IAudioCaptureClient* captureClient = nullptr;

    CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&enumerator
    );

    // Get default output device (speakers)
    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);

    device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        NULL,
        (void**)&audioClient
    );

    WAVEFORMATEX* format = nullptr;
    audioClient->GetMixFormat(&format);

    audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,   // KEY FLAG
        10000000,
        0,
        format,
        NULL
    );

    audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&captureClient
    );

    audioClient->Start();

    while (true) {
        UINT32 packetLength = 0;
        captureClient->GetNextPacketSize(&packetLength);

        while (packetLength > 0) {
            BYTE* data;
            UINT32 frames;
            DWORD flags;

            captureClient->GetBuffer(&data, &frames, &flags, NULL, NULL);

            int bytes = frames * format->nBlockAlign;

            std::cout << "Captured " << bytes << " bytes\n";

            // Send to server here

            captureClient->ReleaseBuffer(frames);

            captureClient->GetNextPacketSize(&packetLength);
        }

        Sleep(5);
    }
}
```

What this produces:

```
Raw PCM audio from whatever is playing on the computer
```

---

# 4. Sending the Audio to Your Server

You simply stream each audio buffer over a socket.

Windows networking uses **WinSock**.

Example sending captured audio:

```cpp
send(sock, (const char*)data, bytes, 0);
```

So your loop becomes:

```
Capture audio
↓
Get PCM frames
↓
Send frames to server
```

---

# 5. Server Example (Python)

Minimal TCP receiver:

```python
import socket

s = socket.socket()
s.bind(("0.0.0.0",9000))
s.listen(1)

conn, addr = s.accept()
print("connected")

while True:
    data = conn.recv(4096)
    if not data:
        break

    # do something with audio data
```

---

# 6. Major Problem: Raw PCM Is Huge

Raw audio bandwidth:

```
48kHz * 2 channels * 16 bits
≈ 192 KB/sec
```

This becomes very expensive over the internet.

Professional solutions compress audio using:

* **Opus (audio codec)** – best for real-time
* **AAC**

Example pipeline used by real apps:

```
WASAPI Loopback
↓
PCM
↓
Opus Encoder
↓
UDP packets
↓
Server
```

---

# 7. Real Production Architecture

For stable streaming you should run **two threads**.

```
Thread 1:
    capture audio

Thread 2:
    send network packets
```

Using a queue:

```
Capture → Queue → Network Sender
```

This prevents audio glitches if the network stalls.

---

# 8. A Much Easier Cross-Platform Alternative

Instead of raw WASAPI code, many developers use:

* **PortAudio**
* **RtAudio**

These libraries handle:

* audio devices
* buffering
* cross-platform support

---

# 9. Simple Explanation

Your program will basically do this:

1. Listen to the **system audio output**.
2. Receive small chunks of sound samples.
3. Send those chunks to a server using sockets.

So the loop looks like:

```
while (running)
{
    capture sound chunk
    send chunk to server
}
```

---

💡 **Important improvement I recommend**

If you want **real-time streaming that works well over the internet**, you should use:

```
WASAPI loopback
↓
Opus encoder
↓
UDP streaming
```

This reduces bandwidth **~10×** and keeps latency low.
