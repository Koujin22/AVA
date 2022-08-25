from ctypes import FormatError
import logging
from threading import Timer
from typing import Tuple
from .AvaConnection import AvaConnection


class BadSocketState(Exception):
    pass

class TerminateModule(Exception):
    pass

class AvaModule():

    connection: AvaConnection

    module_name: str
    intent_name: str

    ava_log: logging.Logger
    log: logging.Logger

    __can_send_req: bool = False
    __can_listen_req: bool = False

    def __init__(self, module_name: str, intent_name: str, log_level: int = logging.INFO):
        
        self.intent_name = intent_name
        self.module_name = module_name
        
        ch = logging.StreamHandler()
        formatter = logging.Formatter(
            '|%(levelname)7s| %(name)22s | %(message)s')
        ch.setFormatter(formatter)
        self.ava_log = logging.getLogger("AvaModule")
        self.ava_log.setLevel(log_level)
        self.ava_log.addHandler(ch)

        ch = logging.StreamHandler()
        formatter = logging.Formatter(
            '|%(levelname)7s| %(name)22s | %(message)s')
        ch.setFormatter(formatter)
        self.log = logging.getLogger(module_name)
        self.log.setLevel(logging.INFO)
        self.log.addHandler(ch)

        self.ava_log.info("Creating zmq context and sockets")
        self.connection = AvaConnection(module_name, intent_name)
        self.ava_log.info("Sync completed")

    def __send_req(self, msg: bytes) -> None:
        if (self.__can_send_req):

            self.__can_send_req = False
            self.__can_listen_req = True

            t = Timer(5.0, self._can_listen_timeout)
            t.start()
            self.connection.send_req(msg)
        else:
            raise BadSocketState("Unable to send req before getting intent.")

    def __listen_req(self) -> bytes:
        if (self.__can_listen_req):
            self.__can_listen_req = False
            return self.connection.recv_req()
        else:
            raise BadSocketState("Unable to listen req before sending.")
    
    def getResopnse(self) -> str:
        if(self.__can_listen_req):
            return self.__listen_req().decode()
        else:
            raise BadSocketState("Unable to listen req before sending.")

    def waitForIntent(self) -> Tuple[str, str]:
        self.ava_log.info(
            "Waiting for intent. Intent to listen is: %s", self.intent_name)
        msg = self.connection.recv_sub()
        if(msg == b'MODULES_stop'):
            raise TerminateModule("Ava called to stop")
        self.__can_send_req = True
        t = Timer(5.0, self._can_send_timeout)
        t.start()
        self.ava_log.info("Intent recieved. Intent: %s", msg)
        return msg.decode().split(" ", 1)

    def say(self, msg: str, autoListen: bool = True) -> None:
        self.ava_log.info("Sending to ava: say_%s", msg)
        self.__send_req(bytes("say_"+msg, 'ascii'))
        if(autoListen):
            self.__listen_req()

    def sayAndListen(self, msg: str) -> str:
        self.ava_log.info("Sending to ava: listen_%s", msg)
        self.__send_req(bytes("say-listen_"+msg, 'ascii'))
        response: bytes = self.__listen_req()

        self.ava_log.info("Recieved from ava: %s", response)
        return response.decode()

    def _can_send_timeout(self):
        if (self.__can_send_req):
            self.ava_log.warn(
                "Timeout on send. Toggling send to off.")
        self.__can_send_req = False

    def _can_listen_timeout(self):
        if (self.__can_listen_req):
            self.ava_log.warn(
                "Timeout on listen req socket. Toggling listen to off.")
            msg = self.connection.recv_req(2)
            self.ava_log.debug("Msg: %s", msg)
        self.__can_listen_req = False
    

        
