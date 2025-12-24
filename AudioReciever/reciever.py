import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind(('0.0.0.0', 5000))

print("waiting for message")

while True:
    data, addr = sock.recvfrom(1024)
    message = data.decode('utf-8')
    print(f"The message was: {message}")
