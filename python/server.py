import json
from threading import Thread
from ipaddress import ip_address
from socket import socket
from socket import AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR

# TODO: Thread pool

OK = 0
INVALID_METHOD = 1
INVALID_HOST = 2
INVALID_JSON = 3


class ValidationError(Exception):
    def __init__(self, code, msg):
        self.code = code
        self.msg = msg


class Answer:
    def __repr__(self):
        return f"{self.__class__.__name__}(code={self.code}, msg={self.msg})"

    def __init__(self, code, msg):
        self.code = code
        self.msg = msg

    def dump(self):
        data = json.dumps({
            'code': self.code,
            'msg': self.msg,
        })
        return bytes(data, encoding='utf8')


class Request:
    def __init__(self, data):
        self.method, self.addr = self.parse(data)

    @staticmethod
    def parse(data):
        try:
            data = json.loads(data.decode('utf8'))
        except Exception as exc:
            raise ValidationError(INVALID_JSON, f"Json error: {exc}")

        return data.get('method'), data.get('host')


class Executor:
    METHODS = ['append', 'remove']

    def __init__(self, request):
        self.request = request

    def run(self):
        try:
            self.validate(self.request)
            return getattr(self, self.request.method)(self.request.addr)

        except ValidationError as error:
            return Answer(error.code, error.msg)

    def validate(self, request):
        if self.request.method not in self.METHODS:
            raise ValidationError(INVALID_METHOD, f'Method "{self.request.method}" does not exist')
        if self.request.addr is None:
            raise ValidationError(INVALID_HOST, f'Host address must be specified')
        try:
            ip = ip_address(self.request.addr)
        except Exception as exc:
            raise ValidationError(INVALID_HOST, f'Invalid host address "{self.request.addr}"')

    def append(self, addr):
        print(f"iptables -A FORWARD -s {addr} -j ACCEPT")
        return Answer(OK, f"Host {addr} has been succesfully added")

    def remove(self, addr):
        print(f"iptables -D FORWARD -s {addr} -j ACCEPT")
        return Answer(OK, f"Host {addr} has been succesfully removed")


class Connection(Thread):
    def __init__(self, sock, addr):
        self.socket = sock
        self.addr = addr
        super().__init__()

    def run(self):
        print("connection %s:%s started" % self.addr)

        msg = self.socket.recv(1024)
        print(msg)

        executor = Executor(Request(msg))
        ans = executor.run()
        print(ans)

        self.socket.send(ans.dump())
        self.teardown()

    def teardown(self):
        self.socket.close()


class Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.socket = None
        self.connections = []

    def setup(self):
        self.socket = socket(AF_INET, SOCK_STREAM)
        self.socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.socket.bind((self.host, self.port))
        self.socket.listen(1)

    def run(self):
        while True:
            try:
                sock, addr = self.socket.accept()
            except KeyboardInterrupt:
                return

            conn = Connection(sock, addr)
            conn.start()
            self.connections.append(conn)

    def teardown(self):
        for conn in self.connections:
            conn.teardown()

        self.socket.close()

    def start(self):
        self.setup()
        self.run()
        self.teardown()
        

if __name__ == "__main__":
    server = Server(host="localhost", port=5555)
    server.start()
