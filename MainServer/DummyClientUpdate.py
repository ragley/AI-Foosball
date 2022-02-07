import socket
from GameState import GameState
from Message import Message

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 5000       # The port used by the server

try:
    #with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        #s.connect((HOST, PORT))

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    while True:
        message = Message(True, 1)
        sock.sendall(message.encode_message())
        print(message.to_string())
except KeyboardInterrupt:
    print("caught keyboard interrupt, exiting")
finally:
    sock.send(b'')
    sock.close()