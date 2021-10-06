import sys
import json
from socket import socket
from socket import AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR


class Client:
    def __init__(self, addr, port):
        self.addr = addr
        self.port = port

    def pack(self, payload):
        return bytes(json.dumps(payload), encoding='utf8')

    def unpack(self, response):
        return json.loads(response.decode('utf8'))

    def send(self, payload):
        request = self.pack(payload)

        sock = socket(AF_INET, SOCK_STREAM)
        sock.connect((self.addr, self.port))
        sock.send(request)
        response = sock.recv(1024)

        return self.unpack(response)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        sys.stderr.write("""
%s <method> <host>

Methods:
    - append
    - remove

Hosts:
    - Any valid ip address
""".lstrip() % sys.argv[0])
        sys.exit(1)

    payload = {
        'method': sys.argv[1],
        'host': sys.argv[2],
    }

    response = Client('localhost', 5555).send(payload)
    print(response['msg'])
    sys.exit(response['code'])
