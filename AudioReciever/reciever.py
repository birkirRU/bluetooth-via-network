import socket
import subprocess

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind(('0.0.0.0', 5000))

print("waiting for message")

class AudioPacketAssembler:
    def __init__(self):
        self.buffer = bytearray()

    def add_packet(self, clean_packet_data: bytearray):
        self.buffer.extend(clean_packet_data)


def main():

    header, addr = sock.recvfrom(12)
    sample_rate = int.from_bytes(header[0:5], byteorder='little')
    bit_depth   = int.from_bytes(header[5:9], byteorder='little')
    channels    = int.from_bytes(header[9::], byteorder='little')

    period_size = 1024 # frames -> each frame has 4 bytes (float) per channel -> 8192 bytes
    buffer_byte_size = period_size * channels * (bit_depth//8) 
    period_time = ( period_size // sample_rate ) * 1000 # buffer length in ms

    subprocess.Popen(
        args=f"aplay -f FLOAT_LE -r {sample_rate} -c {channels} --period-size={period_size} --buffer-size={period_size*4}",
        stdin=subprocess.PIPE,
        stderr=subprocess.PIPE
    )




    while True:
        max_buffer_size: int = 35280
        packet, addr = sock.recvfrom(max_buffer_size)


main()