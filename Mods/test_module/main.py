
from .._ava_module.Module import Module
from .._ava_module.AvaModule import AvaModule, TerminateModule
import time
import logging

class Test_Module(Module):
    module_name: str = "test"
    
    
    def __init__(self):
        super().__init__(self.module_name, logging.INFO)
        self.ava = AvaModule(self.module_name, "TEST", logging.DEBUG)


    def start(self):
        time.sleep(5)
        resp = self.ava.requestCommunication(1)
        time.sleep(1)
        self.log.info(resp)
        self.ava.finish()



if __name__ == "__main__":
    note: Test_Module = Test_Module()
    try:
        note.start()
    except(TerminateModule):
        pass

    del note
