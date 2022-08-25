from ctypes import FormatError
from threading import Timer
import logging
from typing import Tuple
from .AvaConnection import AvaConnection
from .Module import Module


class BadSocketState(Exception):
    pass

class TerminateModule(Exception):
    pass

class AvaModule(Module):

    connection: AvaConnection

    module_name: str
    intent_name: str

    __can_send_req: bool = False
    __can_listen_req: bool = False

    def __init__(self, module_name: str, intent_name: str, log_level: int = logging.INFO):
        super().__init__("AvaModule", log_level)
        self.intent_name = intent_name
        self.module_name = module_name

        self.log.info("Creating zmq context and sockets")
        self.connection = AvaConnection(module_name, intent_name)
        self.log.info("Sync completed")

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

    def _can_send_timeout(self):
        if (self.__can_send_req):
            self.log.warn(
                "Timeout on send. Toggling send to off.")
        self.__can_send_req = False

    def _can_listen_timeout(self):
        if (self.__can_listen_req):
            self.log.warn(
                "Timeout on listen req socket. Toggling listen to off.")
            msg = self.connection.recv_req(2)
            self.log.debug("Msg: %s", msg)
        self.__can_listen_req = False

    def waitForIntent(self) -> Tuple[str, str]:
        self.log.info(
            "Waiting for intent. Intent to listen is: %s", self.intent_name)
        msg = self.connection.recv_sub()
        if(msg == b'MODULES_stop'):
            raise TerminateModule("Ava called to stop")
        self.__can_send_req = True
        t = Timer(5.0, self._can_send_timeout)
        t.start()

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
        if(self.__can_listen_req):
            return self.__listen_req().decode()
        else:
            raise BadSocketState("Unable to listen req before sending.")

    def say(self, msg: str, lang: str = "en-us", asy: bool = False, autoListen: bool = True) -> None:
        self.log.info("Sending to ava: say_%s", msg)
        self.__send_req(bytes("say_"+msg+"#A"+('t' if asy else 'f')+"L"+lang, 'ascii'))
        if(autoListen):
            self.__listen_req()
            self.__can_send_req = True

    def sayAndListen(self, msg: str, lang: str = "en-us", asy: bool = False) -> str:
        self.log.info("Sending to ava: listen_%s", msg)
        self.__send_req(bytes("say-listen_"+msg+"#A"+('t' if asy else 'f')+"L"+lang, 'ascii'))
        response: bytes = self.__listen_req()

        self.log.info("Recieved from ava: %s", response)
        return response.decode()

