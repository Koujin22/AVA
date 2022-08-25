from ctypes import FormatError
import string
import logging
from threading import Timer
from .AvaConnection import AvaConnection


class BadSocketState(Exception):
    pass


class AvaModule():

    connection: AvaConnection

    module_name: string
    intent_name: string

    ava_log: logging.Logger
    log: logging.Logger

    __can_send_req: bool = False
    __can_listen_req: bool = False

    def __init__(self, module_name: string, intent_name: string, log_level: int = logging.INFO):
        #logging.basicConfig(level=logging.DEBUG, format='%(relativeCreated)6d %(threadName)s %(message)s')
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

            t = Timer(60.0, self._can_listen_timeout)
            self.connection.send_req(msg)
        else:
            raise BadSocketState("Unable to send req before getting intent.")

    def __listen_req(self) -> bytes:
        if (self.__can_listen_req):
            self.__can_listen_req = False
            return self.connection.recv_req()
        else:
            raise BadSocketState("Unable to listen req before sending.")

    def waitForIntent(self) -> string:
        self.ava_log.info(
            "Waiting for intent. Intent to listen is: %s", self.intent_name)
        msg = self.connection.recv_sub()
        self.__can_send_req = True
        t = Timer(60.0, self._can_send_timeout)
        self.ava_log.info("Intent recieved. Intent: %s", msg)
        return msg.decode()

    def say(self, msg: string) -> None:
        self.ava_log.info("Sending to ava: say_%s", msg)
        self.__send_req(bytes("say_"+msg, 'ascii'))

    def sayAndListen(self, msg: string) -> string:
        self.ava_log.info("Sending to ava: listen_%s", msg)
        self.__send_req(bytes("say-listen_"+msg, 'ascii'))
        response: bytes = self.__listen_req()

        self.ava_log.info("Recieved from ava: %s", response)
        return response.decode()

    def _can_send_timeout(self):
        if (self.__can_send_req):
            self.ava_log.warn(
                "Timeout on send. Toggling send to off. Modules still need to send empty msgs to AvA")
        self.__can_send_req = False

    def _can_listen_timeout(self):
        if (self.__can_listen_req):
            self.ava_log.warn(
                "Timeout on listen req socket. Toggling listen to off. Modules still need to listen for empty msgs from AvA")
        self.__can_listen_req = False
