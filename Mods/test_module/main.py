import time
from .._ava_module.AvaModule import AvaModule, TerminateModule
from .._ava_module.Module import Module
import logging


class Test_Module(Module):

    module_name: str = "test_module"

    ava: AvaModule

    def __init__(self):
        super().__init__(self.module_name, logging.INFO)
        self.ava = AvaModule(self.module_name, "TEST", logging.INFO)

    def start(self):
        self.log.info("Iniciado!")
        time.sleep(5)
        self.ava.startCommunication(1)
        count = 5
        while(count > 0):
            self.ava.say("T")
            time.sleep(1)
            count-=1
        self.ava.finish()

if __name__ == "__main__":
    note: Test_Module = Test_Module()
    try:
        note.start()
    except(TerminateModule):
        pass

    del note

