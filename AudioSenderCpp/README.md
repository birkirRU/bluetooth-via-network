# Audio Streaming Client - Build Instructions

## Prerequisites

### Windows Requirements
- Windows 10/11
- Visual Studio 2022 or MinGW-w64
- CMake 3.15+
- Opus library (libopus)

### Install Opus on Windows

#### Option 1: vcpkg
```bash
vcpkg install opus:x64-windows
```

#### Option 2: Pre-built binaries
Download from https://opus-codec.org/downloads/ or use MSYS2:
```bash
pacman -S mingw-w64-x86_64-opus
```

## Build Instructions

### Using CMake with MSVC (Visual Studio)

```bash
# Configure
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake

# Build
cmake --build build --config Release

# Run
.\build\Release\AudioStreamClient.exe
```

### Using CMake with MinGW

```bash
# Configure
cmake -B build -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=mingw32-make

# Build
cmake --build build

# Run
.\build\AudioStreamClient.exe
```

### Using MSVC directly (without CMake)

```bash
# Compile
cl /EHsc /O2 /std:c++17 /I<opus_include_path> /Fe:AudioStreamClient.exe ^
   main.cpp audio_capture.cpp opus_encoder.cpp network_sender.cpp ^
   /link /LIBPATH:<opus_lib_path> opus.lib ws2_32.lib ole32.lib uuid.lib winmm.lib
```

### Using MinGW directly

```bash
g++ -std=c++17 -O2 -o AudioStreamClient.exe ^
    main.cpp audio_capture.cpp opus_encoder.cpp network_sender.cpp ^
    -I<opus_include_path> -L<opus_lib_path> -lopus -lws2_32 -lole32 -luuid -lwinmm
```

## Configuration

Edit `network_sender.h` to configure the server:
```cpp
constexpr int SERVER_PORT = 5000;
constexpr const char* SERVER_IP = "192.168.1.100";  // Your Raspberry Pi IP
```

Or pass via CMake:
```bash
cmake -B build -DSERVER_IP="192.168.1.100" -DSERVER_PORT=5000
```

## Packet Format

Each UDP packet contains a header followed by Opus payload:

```
+----------------+----------------+----------------+
|    Header (8 bytes)    |   Opus Payload (variable)  |
+----------------+----------------+----------------+
```

### PacketHeader Structure (8 bytes)

| Field           | Type    | Size   | Description                    |
|-----------------|---------|--------|--------------------------------|
| sequence_number | uint32  | 4      | Packet sequence number        |
| timestamp       | uint32  | 4      | Audio timestamp (sample count) |
| payload_size    | uint16  | 2      | Size of Opus payload          |

All multi-byte fields are **little-endian**.

### Decoding on Raspberry Pi (C)

```c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t sequence_number;
    uint32_t timestamp;
    uint16_t payload_size;
} PacketHeader;
#pragma pack(pop)

#define MAX_PAYLOAD_SIZE 4000

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return 1;

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5000);

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sock);
        return 1;
    }

    printf("Listening on port 5000...\n");

    while (1) {
        uint8_t buffer[sizeof(PacketHeader) + MAX_PAYLOAD_SIZE];
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        ssize_t received = recvfrom(
            sock,
            buffer,
            sizeof(buffer),
            0,
            (struct sockaddr*)&clientAddr,
            &addrLen
        );

        if (received < sizeof(PacketHeader)) {
            continue;
        }

        PacketHeader* header = (PacketHeader*)buffer;
        uint8_t* opusPayload = buffer + sizeof(PacketHeader);
        uint16_t payloadSize = header->payload_size;

        printf("Seq: %u, TS: %u, Size: %u bytes\n",
               header->sequence_number,
               header->timestamp,
               payloadSize);

        // Decode Opus payload using libopus
        // opus_decode_float(decoder, opusPayload, payloadSize, pcmOutput, frameSize, 0);
    }

    close(sock);
    return 0;
}
```

## Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│ Audio Capture   │     │ Opus Encoder    │     │ Network Sender  │
│ Thread          │     │ Thread          │     │ Thread          │
├─────────────────┤     ├─────────────────┤     ├─────────────────┤
│ WASAPI Loopback │────▶│ Encode PCM to   │────▶│ Send UDP packet │
│ Capture         │     │ Opus            │     │ to Pi           │
└─────────────────┘     └─────────────────┘     └─────────────────┘
        │                       │                       │
        ▼                       ▼                       ▼
   PCM Queue             Packet Queue            Network
```

## Audio Parameters

- Sample Rate: 48000 Hz
- Channels: 2 (stereo)
- Bit Depth: 32-bit float
- Frame Size: 20 ms (960 samples per frame)
- Bitrate: 64 kbps (configurable)
- Application: RESTRICTED_LOWDELAY (for real-time streaming)
