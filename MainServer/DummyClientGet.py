import socket
import time
from GameState import GameState
from Message import Message

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 5000       # The port used by the server

try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    while True:
        message = Message(False, 1)
        start = time.perf_counter()
        sock.sendall(message.encode_message())

        recv_data = sock.recv(1024)
        returned = Message()
        returned.decode_header(recv_data[:5])
        returned.data_bytes = recv_data[5:]
        print(returned.to_string())
        print(returned.decode_data())
        print(time.perf_counter() - start)
except KeyboardInterrupt:
    print("caught keyboard interrupt, exiting")
finally:
    sock.send(b'')
    sock.close()