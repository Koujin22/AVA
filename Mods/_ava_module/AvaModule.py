from ctypes import FormatError
from enum import Enum
from threading import Timer
import logging
from typing import Tuple
from .AvaConnection import AvaConnection
from .Module import Module


class BadSocketState(Exception):
    pass

class TerminateModule(Exception):
    pass

class InvalidArguments(Exception):
    pass

class CouldntConfirm(Exception):
    pass

class SocketState(Enum):
    WAITING_INTENT = 0
    READY_TO_SEND = 1
    READY_TO_LISTEN = 2

class AvaModule(Module):

    connection: AvaConnection

    module_name: str
    intent_name: str

    socket_state: SocketState
    timeout: Timer

    def __init__(self, module_name: str, intent_name: str, log_level: int = logging.INFO):
        super().__init__("AvaModule", log_level)
        self.intent_name = intent_name
        self.module_name = module_name
        self.log.info("Starting connection...")
        self.connection = AvaConnection(module_name, intent_name)
        self.log.info("Connection stablished!")
        self.socket_state = SocketState.WAITING_INTENT

    def __reset_timeout(self, time: float, cancel: bool = True):
        if(cancel):
            self.timeout.cancel()
        self.timeout = Timer(time, self._socket_timeout)
        self.timeout.start()

    def __send_req(self, msg: bytes, time: float = 5.0) -> None:
        
        if (self.socket_state == SocketState.READY_TO_SEND):
            self.log.debug("Sending msg to req socekt. msg: %s", msg)
            self.connection.send_req(msg)
            self.__reset_timeout(time)
            self.socket_state = SocketState.READY_TO_LISTEN
        else:
            raise BadSocketState("Unable to send req before getting intent.")

    def __listen_req(self, time: float = 5.0) -> bytes:
        
        if (self.socket_state == SocketState.READY_TO_LISTEN):
            self.log.debug("Listening for msg on req socekt.")
            msg = self.connection.recv_req()
            self.log.debug("Got msg. Msg: %s", msg)
            if(msg == b'done'):
                self.timeout.cancel()
                self.socket_state = SocketState.WAITING_INTENT
            else:
                self.socket_state = SocketState.READY_TO_SEND
                self.__reset_timeout(time)
                return msg
        else:
            raise BadSocketState("Unable to listen req before sending.")

    def __listen_sub(self) -> bytes:
        if(self.socket_state == SocketState.WAITING_INTENT):
            self.log.debug("Listening msg from sub socket.")
            msg =  self.connection.recv_sub()
            self.log.debug("Got msg from sub. Msg: %s", msg)
            if(msg == b'MODULES_stop'):
                raise TerminateModule("Ava called to stop")
            else:
                self.socket_state = SocketState.READY_TO_SEND
                self.__reset_timeout(5.0, False)
                return msg
        else:
            raise BadSocketState("Unable to listen sub before ending conversation")

    def _socket_timeout(self):
        if(self.socket_state == SocketState.READY_TO_SEND):
            self.log.warn("Timeout on socket. Reseting connection.")
            self.socket_state = SocketState.WAITING_INTENT
        elif(self.socket_state == SocketState.READY_TO_LISTEN):
            self.__listen_req()
            self.socket_state = SocketState.WAITING_INTENT
            self.log.warn("Timeout on socket. Reseting connection.")
        else:
            self.log.debug("Timeout reached, its waiting for intent")

    def waitForIntent(self) -> Tuple[str, str]:
        self.log.info("Waiting for intent. Intent to listen is: %s", self.intent_name)
        
        msg = self.__listen_sub()

        self.log.info("Intent recieved. Intent: %s", msg)
        module, action, slotsstr = msg.decode().split(" ", 2)

        slots = {}
        for slot in slotsstr.split(","):
            if (len(slot) == 0):
                continue
            else:
                key, val = slot.split(":")
                slots[key] =val

        return module, action, slots

    def getResopnse(self) -> str:
        msg = self.__listen_req().decode()
        self.log.info("Got msg: %s", msg)
        return msg

    def finish(self) -> None:
        self.log.info("Finishing coms.")
        self.timeout.cancel()
        self.__send_req(b'done')
        self.__listen_req()

    def say(self, msg: str, lang: str = "en-us", asy: bool = False, autoListen: bool = True) -> None:
        self.log.info("Saying: %s | Lang: %s | async: %s | autolisten: %s ", msg, lang, asy, autoListen)
        self.__send_req(bytes("say_"+msg+"#A"+('t' if asy else 'f')+"L"+lang, 'UTF-8'), max(len(msg)/5, 5) if not asy else 5)
        if(autoListen):
            self.__listen_req()

    def sayAndListen(self, msg: str, lang: str = "en-us", asy: bool = False, listen_time: int = 10) -> str:
        self.log.info("Saying and listening: %s | Lang: %s | async: %s ", msg, lang, asy)
        self.__send_req(bytes("say-listen_"+msg+"#A"+('t' if asy else 'f')+"L"+lang+"T"+str(listen_time), 'UTF-8'), len(msg)/10+listen_time+5)
        response: bytes = self.__listen_req()
        self.log.debug("Got dictation: %s", response)
        return response.decode()

    def confirmation(self, msg: str, lang: str = "en-us", asy: bool = False) -> bool:
        self.log.info("Getting confirmation: %s | Lang: %s | async: %s ", msg, lang, asy)
        self.__send_req(bytes("confirm_"+msg+"#A"+('t' if asy else 'f')+"L"+lang, 'UTF-8'), len(msg)/9+5)
        response: bytes = self.__listen_req()
        self.log.debug("Got dictation: %s", response)
        if(response == b''):
            raise CouldntConfirm
        msg = response.decode()
        return msg == 'yes'