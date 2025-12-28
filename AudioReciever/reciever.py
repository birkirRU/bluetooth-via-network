import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind(('0.0.0.0', 5000))

print("waiting for message")

while True:
    max_buffer_size: int = 35280
    data, addr = sock.recvfrom(max_buffer_size)
    print(f"Bytes")
    print()
