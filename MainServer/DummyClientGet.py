import socket
import time
from Message import Message

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 5000       # The port used by the server
i = 0
total_time = 0.0

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    while True:
        message = Message("GET")
        message.request_rods()
        start = time.perf_counter()
        sock.sendall(message.encode_to_send(True))
        recv_data = sock.recv(1024)
        received = Message("RECEIVED")
        received.decode_from_receive(recv_data)
        print(received.data)
        total_time += time.perf_counter() - start
        i += 1
        print(total_time / i)
except KeyboardInterrupt:
    print("caught keyboard interrupt, exiting")
finally:
    sock.send(b'')
    sock.close()