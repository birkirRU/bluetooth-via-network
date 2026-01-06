import socket
import subprocess
import time
from itertools import islice
from collections import deque
import threading

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 5000))


class JitterBuffer:
    def __init__(self, period_size, buffer_size, sample_rate, channels, bit_depth):
        self.max_buffer_size = buffer_size
        self.buffer = deque(maxlen=buffer_size)

        self.ps = period_size
        self.period_bytes = period_size * channels * (bit_depth//8) 
        self.period_time = period_size // sample_rate # in seconds

        self.write_index = 0
        self.read_index = 0
        self.buffer_filled = 0
        
        self.init_time = time.perf_counter()

        print(f"Jitter Buffer initialized:")
        print(f"  Period: {period_size} frames = {self.period_time:.1f}ms")
        print(f"  Buffer: {buffer_size} periods = {buffer_size * self.period_time:.1f}ms")
        print(f"  Bytes per period: {self.period_bytes}")

    def _chunk_packet(data_list: bytearray, size):
        """Batch data into tuples of length n. The last batch may be shorter."""
        it = iter(data_list)
        while batch := bytes(islice(it, size)):
            yield batch

        
    def _write_buffer(self):
        pass

    def _read_buffer(self):
        pass
        
    def reader_loop(self):
        pass

    def writer_loop(self):

        prev_scarce_period = b''
        while self.running:

            max_packet_size: int = 35280
            packet, addr = sock.recvfrom(max_packet_size)
            first_period_bytes = self.period_bytes - len(prev_scarce_period) + 1
            self.buffer.append(prev_scarce_period + packet[0:first_period_bytes])
            
            for period in self._chunk_packet(packet[self.period_bytes::], self.period_bytes):
                if len(period) == self.period_bytes:
                    self.buffer.append(period)
                else:
                    prev_scarce_period = period

            
    def start(self):
        self.running = True


        writer_thread = threading.Thread(target=self.writer_loop, daemon=True)
        writer_thread.start()

        while True:
            if len(self.buffer) < self.max_buffer_size//2:
                read_thread = threading.Thread(target=self.reader_loop, daemon=True)
                read_thread.start()
                break


def main():

    header, addr = sock.recvfrom(12)
    sample_rate = int.from_bytes(header[0:5], byteorder='little') # 48kHz
    bit_depth   = int.from_bytes(header[5:9], byteorder='little') # 32 bit
    channels    = int.from_bytes(header[9::], byteorder='little') # 2 sterio

    period_size = 1024 # frames -> each frame has 4 bytes (float) per channel -> 8192 bytes

    subprocess.Popen(
        args=f"aplay -f FLOAT_LE -r {sample_rate} -c {channels} --period-size={period_size} --buffer-size={period_size*4}",
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    jitterbuffer = JitterBuffer(
        period_size=period_size,
        buffer_size=8, # periods
        sample_rate=sample_rate,
        channels=channels,
        bit_depth=bit_depth
    )


    jitterbuffer.start()

main()