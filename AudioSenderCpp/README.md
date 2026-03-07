# Audio Streaming Client - Build Instructions

## Prerequisites

### Windows Requirements
- Windows 10/11
- **MSYS2** with UCRT64 environment (NOT MSVC/Visual Studio)
- CMake 3.15+

### Install MSYS2

1. Download and install from https://www.msys2.org/
2. Use default installation path: `C:\msys64`

### Install Required Packages

Open **MSYS2 UCRT64** terminal (not MSYS, not MINGW64) and run:

```bash
pacman -S --noconfirm mingw-w64-ucrt-x86_64-opus
pacman -S --noconfirm mingw-w64-ucrt-x86_64-cmake
pacman -S --noconfirm mingw-w64-ucrt-x86_64-ninja
```

This installs:

| Package | Version | Description |
|---------|---------|-------------|
| mingw-w64-ucrt-x86_64-opus | 1.5.2 | Audio codec library |
| mingw-w64-ucrt-x86_64-cmake | 4.2.1 | Build system generator |
| mingw-w64-ucrt-x86_64-ninja | 1.13.2 | Fast build tool |
| mingw-w64-ucrt-x86_64-gcc | 15.2.0 | C/C++ compiler (usually pre-installed) |

### Tool Locations

After installation, tools are located at:
- **Compiler**: `C:\msys64\ucrt64\bin\g++.exe`
- **CMake**: `C:\msys64\ucrt64\bin\cmake.exe`
- **Ninja**: `C:\msys64\ucrt64\bin\ninja.exe`
- **Opus headers**: `C:\msys64\ucrt64\include\opus\opus.h`
- **Opus library**: `C:\msys64\ucrt64\lib\libopus.a`

## Build Instructions

### Quick Build (Windows Command Prompt)

```cmd
cd AudioSenderCpp
build.bat
```

### Manual Build (Windows Command Prompt)

```cmd
set PATH=C:\msys64\ucrt64\bin;%PATH%

cmake -G "Ninja" ^
  -DCMAKE_CXX_COMPILER=C:\msys64\ucrt64\bin\g++.exe ^
  -DCMAKE_C_COMPILER=C:\msys64\ucrt64\bin\gcc.exe ^
  -DCMAKE_MAKE_PROGRAM=C:\msys64\ucrt64\bin\ninja.exe ^
  -B build -S .

cmake --build build
```

### Build Output

- **Executable**: `build/AudioStreamClient.exe`
- **Size**: ~400 KB

## Configuration

Edit `network_sender.h` to configure the server:
```cpp
constexpr int SERVER_PORT = 5000;
constexpr const char* SERVER_IP = "192.168.1.100";  // Your receiver IP
```

## CMake Configuration Details

The `CMakeLists.txt` finds Opus in this order:
1. **pkg-config** - If available
2. **MSYS2 UCRT64** - `C:/msys64/ucrt64/include` and `C:/msys64/ucrt64/lib`
3. **MSYS2 MinGW64** - `C:/msys64/mingw64/include` and `C:/msys64/mingw64/lib`
4. **vcpkg** (optional) - `C:/vcpkg/installed/x64-mingw-dynamic/`

### Windows Libraries Linked

These are automatically linked via CMake:
- `ws2_32` - Winsock networking
- `ole32` - COM support
- `uuid` - COM support
- `winmm` - Windows multimedia

## Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│ Audio Capture   │     │ Opus Encoder    │     │ Network Sender  │
│ Thread          │     │ Thread          │     │ Thread          │
├─────────────────┤     ├─────────────────┤     ├─────────────────┤
│ WASAPI Loopback │────▶│ Encode PCM to   │────▶│ Send UDP packet │
│ Capture         │     │ Opus            │     │ to receiver     │
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
- Application: RESTRICTED_LOWDELAY (real-time streaming)

## Packet Format

Each UDP packet contains:

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

## Troubleshooting

### "Opus not found"
```bash
pacman -S mingw-w64-ucrt-x86_64-opus
```

### "CMAKE_MAKE_PROGRAM is not set" or "Ninja not found"
```bash
pacman -S mingw-w64-ucrt-x86_64-ninja
```

### "g++ not found"
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```

### Build fails with "MinGW Makefiles" error
Remove the build folder and rebuild:
```cmd
rmdir /s /q build
build.bat
```

## Notes

- This is a **C++ implementation** (not .NET/C#)
- Uses Windows WASAPI directly (`<mmdeviceapi.h>`, `<audioclient.h>`) - NOT NAudio
- Compiles **natively on Windows** using MinGW, no cross-compilation needed
- Does NOT require Visual Studio or MSVC
