from .._ava_module.AvaModule import AvaModule, TerminateModule
import logging


class Note_taker():

    ava: AvaModule

    def __init__(self):
        self.ava = AvaModule("note_taker", "TODO", logging.DEBUG)

    def start(self):
        while(True):
            module, action = self.ava.waitForIntent()
            self.doAction(action)

    def doAction(self, action: str):
        if(action == 'add'):
            dictation = self.ava.sayAndListen("What do you want me to write down?")
            self.ava.log.info("Got dictation: %s", dictation)
        elif (action == 'get'):
            self.ava.say("On it, getting your to-do list")
        elif (action == 'remove'):
            self.ava.say("What to-do do you want me to remove?")
        else:
            self.ava.say("Sorry this action is not yet implemented")
        

    def __del__(self):
        self.ava.log.info("Deleting")


if __name__ == "__main__":
    note: Note_taker = Note_taker()
    try:
        note.start()
    except(TerminateModule):
        pass

    del note

    # note.waitForIntent()
