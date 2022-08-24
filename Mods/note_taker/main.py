from ..ava_module.AvaModule import AvaModule
class Note_taker(AvaModule):
    
    def __init__(self):
        super().__init__("note_taker", "TODO")

if __name__ == "__main__":
    print("Hola")
    note:Note_taker = Note_taker()

    #note.waitForIntent()
    note.say("Hola mucho gusto")

