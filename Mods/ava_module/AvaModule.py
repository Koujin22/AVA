import string
import time
import zmq
from abc import ABC, abstractmethod


class BadHandShake(Exception):
    pass

class AvaModule(ABC):
    context: zmq.Context
    req_socket: zmq.Socket
    sub_socket: zmq.Socket

    module_name: string
    intent_name: string

    def __init__ (self, module_name: string, intent_name: string ):
        self.context = zmq.Context()
        
        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect("tcp://127.0.0.1:5500")
        self.sub_socket.setsockopt(zmq.SUBSCRIBE, bytes(intent_name, 'ascii'))
        
        time.sleep(1)
        
        self.req_socket = self.context.socket(zmq.REQ)
        self.req_socket.connect("tcp://127.0.0.1:5501")

        self.req_socket.send(bytes(module_name+":syn#"+intent_name, "ascii"))

        
        msg = self.req_socket.recv()

        if msg != b'akg':
            raise BadHandShake("Didnt recv akg")

    def waitForIntent(self) -> string:
        msg = self.sub_socket.recv()
        return msg.decode()

    def say(self, msg:string) -> None:
        self.req_socket.send(bytes("say_"+msg, 'ascii'))

    def sayAndListen(self, msg: string) -> string:
        msg = self.req_socket.send(bytes("say-listen_"+msg, 'ascii'))
        return msg.decode()
    

