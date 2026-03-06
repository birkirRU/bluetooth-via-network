# AGENTS.md - Agentic Coding Guidelines

This document provides guidelines for agents working on this codebase.

## Project Overview

This is a **bluetooth-via-network** project that streams system audio over the network:
- **AudioSender** (C#/.NET 10) - Captures Windows system audio using NAudio (WASAPI loopback)
- **AudioReciever** (Python 3) - Receives and plays audio on Linux using ALSA

## Build Commands

### C# / .NET (AudioSender)

```bash
# Build the project
dotnet build AudioSender/AudioSender.csproj

# Run the sender
dotnet run --project AudioSender/AudioSender.csproj

# Build release
dotnet publish AudioSender/AudioSender.csproj -c Release

# Run a single test (if tests exist)
dotnet test --filter "FullyQualifiedName~TestName"
```

### Python (AudioReciever)

```bash
# Run the receiver (Python 3)
python3 AudioReciever/reciever.py

# Install dependencies (if needed)
pip install -r requirements.txt  # If requirements.txt exists
```

### Running Single Tests

For .NET, use test filtering:
```bash
# Run specific test by name
dotnet test --filter "FullyQualifiedName~ClassName.MethodName"

# Run tests in specific class
dotnet test --filter "FullyQualifiedName~Namespace.ClassName"
```

## Code Style Guidelines

### C# (.NET) Style

#### Formatting
- Use 4 spaces for indentation (no tabs)
- Use **K&R style** bracing: opening brace on same line
- Maximum line length: 120 characters
- Use `var` for type inference when type is obvious
- Use expression-bodied members where appropriate

#### Naming Conventions
- **Classes/Interfaces**: PascalCase (`WasapiLoopbackCapture`)
- **Methods**: PascalCase (`StartRecording`, `GetNextPacketSize`)
- **Properties**: PascalCase (`CaptureState`, `WaveFormat`)
- **Private fields**: camelCase with underscore prefix (`_captureClient`)
- **Constants**: PascalCase (`MaxBufferSize`)
- **Parameters**: camelCase (`audioData`, `ipEndpoint`)

#### Imports
- Use implicit usings (enabled in project)
- Order: System namespaces first, then third-party
- No redundant imports

#### Types
- Enable nullable reference types (`<Nullable>enable</Nullable>`)
- Prefer `string` over `String`
- Use `int` for indices, `uint` for sizes where appropriate

#### Error Handling
- Use try-catch for recoverable errors
- Log exceptions before re-throwing
- Use meaningful exception messages

#### Example C# Code
```csharp
using System.Net;
using System.Net.Sockets;
using NAudio.Wave;

public class AudioSender
{
    private readonly UdpClient _client;
    private WasapiLoopbackCapture? _capture;

    public void Start(string ipAddress, int port)
    {
        try
        {
            var endpoint = new IPEndPoint(IPAddress.Parse(ipAddress), port);
            _capture = new WasapiLoopbackCapture();
            
            _capture.DataAvailable += OnDataAvailable;
            _capture.StartRecording();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to start capture: {ex.Message}");
            throw;
        }
    }

    private void OnDataAvailable(object? sender, WaveInEventArgs e)
    {
        _client.Send(e.Buffer, e.BytesRecorded, _endpoint);
    }
}
```

### Python Style

#### Formatting
- Use 4 spaces for indentation (PEP 8)
- Maximum line length: 100 characters
- Use Black formatting if available

#### Naming Conventions
- **Classes**: PascalCase (`JitterBuffer`)
- **Functions/Methods**: snake_case (`writer_loop`, `start`)
- **Constants**: SCREAMING_SNAKE_CASE (`MAX_BUFFER_SIZE`)
- **Private methods**: underscore prefix (`_write_buffer`)

#### Imports
- Standard library first, then third-party, then local
- Use explicit imports (no `from module import *`)
- Group: stdlib, third-party, local

#### Types
- Use type hints for function signatures
- Use `Optional[T]` instead of `Union[T, None]`
- Use `list`, `dict` instead of `List`, `Dict` (PEP 585)

#### Error Handling
- Catch specific exceptions
- Include context in error messages
- Use logging instead of print for errors

#### Example Python Code
```python
import socket
import threading
from collections import deque
from typing import Optional


class JitterBuffer:
    def __init__(
        self,
        period_size: int,
        buffer_size: int,
        sample_rate: int,
        channels: int,
        bit_depth: int
    ) -> None:
        self.max_buffer_size: int = buffer_size
        self.buffer: deque = deque(maxlen=buffer_size)
        self.period_bytes: int = period_size * channels * (bit_depth // 8)
        self.running: bool = False

    def start(self) -> None:
        """Start the jitter buffer writer thread."""
        self.running = True
        writer_thread = threading.Thread(target=self._writer_loop, daemon=True)
        writer_thread.start()

    def _writer_loop(self) -> None:
        """Internal loop to receive and buffer audio packets."""
        while self.running:
            try:
                packet, _addr = sock.recvfrom(self.period_bytes * 2)
                self.buffer.append(packet)
            except socket.error as e:
                print(f"Socket error: {e}")
                break
```

## Common Patterns

### Network Communication
- Use UDP for low-latency audio streaming
- Send audio format header (sample rate, bit depth, channels) before streaming
- Handle connection failures gracefully

### Audio Processing
- Use appropriate buffer sizes for latency
- Implement jitter buffering for network variability
- Clean up audio resources (dispose capture clients)

## Git Workflow

- Create feature branches for new features
- Write meaningful commit messages
- Test changes before committing
- Follow existing code style in modifications

## Notes

- This project is under active development
- The C# sender uses NAudio 2.2.1 for WASAPI loopback capture
- The Python receiver uses ALSA (aplay) for audio playback
- Network protocol uses simple UDP with header-first transmission
