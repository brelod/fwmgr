import json
from socket import socket
from socket import AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR


class Client:
    def __init__(self, addr, port):
        self.addr = addr
        self.port = port
        self.socket = None

    def run(self, msg):
        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect((self.addr, self.port))
        sock.send(msg)
        ans = sock.recv(1024)
        return ans


if __name__ == "__main__":
    client = Client('localhost', 5555)
    msg = json.dumps({
        'method': 'append',
        'host': '1.2.3.4',
    })
    ans = client.run(bytes(msg, encoding='utf8'))
    print(ans)
