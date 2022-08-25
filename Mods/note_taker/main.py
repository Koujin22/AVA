from ..ava_module.AvaModule import AvaModule


class Note_taker():

    ava: AvaModule

    def __init__(self):
        self.ava = AvaModule("note_taker", "TODO")
        self.ava.say("Hola mucho gusto")


if __name__ == "__main__":
    print("Hola")
    note: Note_taker = Note_taker()

    # note.waitForIntent()
