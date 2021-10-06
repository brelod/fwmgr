"""
Server application for modifying firewall rules with iptables
"""
import re
import sys
import json
import logging
from subprocess import Popen, PIPE
from threading import Thread
from ipaddress import ip_address
from collections import namedtuple
import socket
from socket import AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR
from typing import Tuple, Optional


# Error codes
OK = 0
INVALID_METHOD = 1
INVALID_HOST = 2
INVALID_JSON = 3
EXECUTION_ERROR = 4


logger = logging.getLogger("fwmgr")


class ServerError(Exception):
    def __init__(self, code, msg):
        self.code = code
        self.msg = msg


class ValidationError(ServerError):
    pass


class ExecutionError(ServerError):
    pass


Process = namedtuple("Process", ['return_code', 'stdout', 'stderr'])


class Response:
    def __repr__(self):
        return f"{self.__class__.__name__}(code={self.code}, msg={self.msg})"

    def __init__(self, code: int, msg: str):
        """
        Response class to store and dump server response

        :param code: Error code
        :param msg: Message about what the error code means
        """
        self.code = code
        self.msg = msg

    def dump(self) -> bytes:
        """
        Return with bytes object wich contains json data encoded with utf8
        """
        data = json.dumps({
            'code': self.code,
            'msg': self.msg,
        })
        return bytes(data, encoding='utf8')


class Runner:
    METHODS = ['append', 'remove']

    def __repr__(self):
        return f"{self.__class__.__name__}({self.data})"

    def __init__(self, data: bytes):
        """
        Runner class for executing subprocesses

        :param data: data read from the socket
        """
        self.data = data

    def _run(self, cmd: str) -> Process:
        """
        Execute command via subprocess.Popen()

        :param cmd: command to execute
        :returns: Proces named tuple with return_code, stdout and stderr
        """
        logger.info(f"Execute command: {cmd}")

        process = Popen(cmd.split(), stdout=PIPE, stderr=PIPE)
        stdout, stderr = process.communicate()
        return_code = process.poll()

        if return_code != 0:
            logger.warning(f"Return-code: %s; stdout: %s; stderr: %s", return_code, stdout, stderr)

        return Process(return_code, stdout, stderr)

    def execute(self) -> Response:
        """
        Execute one of the public methods of the Runner class
        Currently supported:
            - append
            - remove

        :returns: with Response object
        """
        try:
            method, addr = self.parse()
            self.validate(method, addr)
            return getattr(self, method)(addr)

        except (ValidationError, ExecutionError) as error:
            return Response(error.code, error.msg)

    def parse(self) -> tuple:
        """
        Parse data of the received from the client and return with the method and the host address.
        Note that both them can be None.
        """
        try:
            data = json.loads(self.data.decode('utf8'))
        except json.JSONDecodeError as error:
            raise ValidationError(INVALID_JSON, f"Json error: {error}")

        return data.get('method'), data.get('host')

    def validate(self, method: str, addr: str) -> None:
        """
        Validate whether the specified method is available in the class and whether the address can be used as a valid
        IP address in iptables commands.

        Raises ValidationError exception on any error
        """
        if method not in self.METHODS:
            raise ValidationError(INVALID_METHOD, f'Method "{method}" does not exist')
        if addr is None:
            raise ValidationError(INVALID_HOST, f'Host address must be specified')
        try:
            ip = ip_address(addr)
        except Exception as exc:
            raise ValidationError(INVALID_HOST, f'Invalid host address "{addr}"')

    def append(self, addr: str) -> Response:
        """
        Append an ACCEPTING iptables rule to the FORWARD chain for the specified host.
        Returns with Response object
        """
        cmd = f"iptables -A FORWARD -s {addr} -j ACCEPT"
        process = self._run(cmd)

        if process.return_code == 0:
            return Response(OK, f"Host {addr} has been successfully appended")

        raise ExecutionError(EXECUTION_ERROR, "Unexpected error happened. Check server logs for further information")

    def remove(self, addr: str) -> Response:
        """
        Remove an iptables rule from the FORWARD chain for the specified host.
        Returns with Response obejct
        """
        cmd = f"iptables -D FORWARD -s {addr} -j ACCEPT"
        process = self._run(cmd)

        if process.return_code == 0:
            return Response(OK, f"Host {addr} has been successfully removed")

        # Handle the errors
        if re.search('does a matching rule exist', process.stderr.decode('utf8')):
            return Response(EXECUTION_ERROR, "No matching rule presents")

        raise ExecutionError(EXECUTION_ERROR, "Unexpected error happened. Check server logs for further information")


class Connection(Thread):
    def __repr__(self):
        return f"{self.__class__.__name__}({self.addr[0]}:{self.addr[1]})"

    def __init__(self, sock: socket.socket, addr: Tuple[str, int], *args, **kwargs):
        """
        Class for representing a connection to a client

        :param sock: socket object which the client can communicate on
        :param addr: tuple like (ip, port)
        """
        self.socket = sock
        self.addr = addr
        super().__init__(*args, **kwargs)

    def run(self) -> None:
        """
        Receive request from client
        Execute command via Runner
        Send response to the client
        """
        logger.debug("Connection received from %s:%s" % self.addr)

        self.socket.settimeout(5)
        try:
            msg = self.socket.recv(1024)
        except socket.timeout:
            logger.warning("Connection timedout to %s", self)
            return self.teardown()

        logger.debug("Request: %s", msg)

        runner = Runner(msg)
        response = runner.execute()
        msg = response.dump()

        self.socket.send(msg)
        logger.debug("Response: %s", msg)

        self.teardown()

    def teardown(self):
        """
        Close the connection to the client
        """
        logger.debug("Connection closed to %s:%s" % self.addr)
        self.socket.close()


class Server:
    def __repr__(self):
        return f"{self.__class__.__name__}({self.host}:{self.port})"

    def __init__(self, host: str, port: int):
        """
        Server class for listening on the specified host:port and manage incoming connections

        :param host: host address to listen on
        :param port: port to listen on
        """
        self.host = host
        self.port = port
        self.socket: Optional[socket.socket] = None

    def setup(self) -> None:
        """
        Setup the socket
        """
        logger.info("Starting server on %s:%s", self.host, self.port)
        self.socket = socket.socket(AF_INET, SOCK_STREAM)
        self.socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.socket.bind((self.host, self.port))
        self.socket.listen(1)
        logger.info("Server has been started")

    def listen(self) -> None:
        """
        Start listening for incomming connections and a threaded connection manager for every client
        """
        assert self.socket is not None, "Socket was not initialized"

        logger.info("Listening for new connections")
        while True:
            try:
                sock, addr = self.socket.accept()
            except KeyboardInterrupt:
                logger.info("Stop listening for new connection because of KeyboardInterrupt")
                return

            conn = Connection(sock, addr)
            conn.start()

    def teardown(self) -> None:
        """
        Close the socket of the server
        """
        assert self.socket is not None, "Socket was not initialized"
        self.socket.close()
        logger.info("Server has been stopped")

    def run(self) -> None:
        """
        Run the server
        """
        self.setup()
        self.listen()
        self.teardown()


if __name__ == "__main__":
    logger.setLevel(logging.INFO)
    handler = logging.StreamHandler(stream=sys.stdout)
    handler.setFormatter(logging.Formatter(fmt='%(asctime)s | %(threadName)10s | %(levelname)7s | %(message)s'))
    logger.addHandler(handler)

    if len(sys.argv) == 2 and (sys.argv[1] == '-d' or sys.argv[1] == '--debug'):
        logger.setLevel(logging.DEBUG)

    server = Server(host="localhost", port=5555)
    server.run()
