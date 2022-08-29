import zmq
import time
import string
class BadHandShake(Exception):
    pass


class AvaConnection():

    context: zmq.Context
    req_socket: zmq.Socket
    sub_socket: zmq.Socket
    init_req_socket: zmq.Socket

    def __init__(self, module_name: str, intent_name: str):
        self.context = zmq.Context()

        self.init_req_socket = self.context.socket(zmq.REQ)
        self.init_req_socket.connect("tcp://127.0.0.1:5502")

        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect("tcp://127.0.0.1:5500")
        self.sub_socket.setsockopt(zmq.SUBSCRIBE, bytes(intent_name, 'ascii'))
        self.sub_socket.setsockopt(zmq.SUBSCRIBE, b'MODULES')
        self.sub_socket.setsockopt(zmq.SUBSCRIBE, bytes(module_name, 'ascii'))

        time.sleep(1)

        self.req_socket = self.context.socket(zmq.REQ)
        self.req_socket.connect("tcp://127.0.0.1:5501")

        self.req_socket.send(bytes(module_name+":syn#"+intent_name, "ascii"))

        msg = self.req_socket.recv()

        if msg != b'akg':
            raise BadHandShake("Didnt recv akg")

    def recv_sub(self) -> bytes:
        return self.sub_socket.recv()

    def send_req(self, msg: bytes) -> None:
        self.req_socket.send(msg)

    def recv_req(self, flag: int = 0) -> bytes:
        return self.req_socket.recv(flag)

    def send_req_com(self, msg: bytes) -> None:
        self.init_req_socket.send(msg)

    def recv_req_com(self, flag: int = 0) -> bytes:
        return self.init_req_socket.recv(flag)
