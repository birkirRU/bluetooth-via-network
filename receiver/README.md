# Network Audio Receiver

C-based UDP audio receiver with jitter buffer for Raspberry Pi.

## Prerequisites

### WSL (Development) Setup

```bash
sudo apt-get update
sudo apt-get install -y build-essential gcc make cmake gdb libopus-dev
```

### Verify Installation

```bash
gcc --version
cmake --version
ls /usr/include/opus/
ls /usr/lib/x86_64-linux-gnu/libopus*
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./audio_receiver [port]
# Default port: 5000
```

## Architecture

```
Thread 1 (Receiver)          Thread 2 (Decoder)
+-------------------+         +-------------------+
| UDP Socket        |         | Jitter Buffer     |
| recvfrom()        |-------> | (5 packets)       |
+-------------------+         +--------+----------+
                                      |
                                      v
                               +------v---------+
                               | Opus Decoder  |
                               +-------+--------+
                                       |
                                       v
                                  stdout (PCM)
```

### Thread 1: UDP Receiver
- Creates UDP socket and binds to port
- Receives packets and parses header
- Pushes to jitter buffer

### Thread 2: Decoder
- Pops packets from jitter buffer in order
- Decodes Opus frames to PCM
- Writes PCM to stdout

## Jitter Buffer

The jitter buffer handles network jitter and packet reordering:

- **Size**: 5 packets
- **Operation**: Ring buffer with sequence tracking
- **Missing packets**: If expected sequence is missing, decoder receives NULL for packet loss concealment

### Algorithm
1. When packet arrives, calculate offset from `next_sequence`
2. If offset < buffer size, store at buffer[offset]
3. When popping, return buffer[0] and shift others
4. If buffer[0] empty, wait with timeout
5. If timeout, return false (trigger PLC)

## Packet Format

```
+----------------+----------------+----------------+
|    Header (8 bytes)    |   Opus Payload (variable)  |
+----------------+----------------+----------------+

Header:
- sequence:  uint32 (packet sequence number)
- timestamp: uint32 (audio timestamp)
- payload_size: uint16 (Opus payload bytes)
```

## Testing

### Send Test Packets

From Windows sender (once implemented):
- Configure SERVER_IP to WSL's IP
- Run sender

### Inspect with tcpdump

```bash
sudo tcpdump -i eth0 -nn port 5000
```

### Debug with GDB

```bash
gdb ./audio_receiver
(gdb) run 5000
# Break on signal handlers, etc.
```

## Future: Adding Audio Output

To add Bluetooth/ALSA output later:

1. Create `audio_output.c` with callback function
2. Replace `fwrite(pcm_out, ...)` with audio callback
3. Use ALSA (`libasound2-dev`) or Bluetooth A2DP

The decoder produces 48kHz stereo 32-bit float PCM frames of 960 samples.
