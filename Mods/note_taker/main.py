from collections import defaultdict
from typing import List, TypedDict
from .._ava_module.AvaModule import AvaModule, TerminateModule
from .._ava_module.Module import Module
from .._storage.mongodb import MongoDB, MongoEntity
from dotenv import load_dotenv
import logging
import os

class Note(MongoEntity):

    topic: str
    note: str

    def __init__(self, topic: str ="", note: str=""):
        self.topic = topic
        self.note = note

    def get(self) -> TypedDict:
        document: TypedDict = {}
        document['topic'] = self.topic
        document['note'] = self.note
        return document

    def query(self) -> TypedDict:
        document: TypedDict = {}
        if(self.topic != ''):
            document['topic'] = self.topic
        if(self.note != ''):
            document['note'] = self.note
        return document

    @classmethod
    def groupByTopic(cls, documents: List[TypedDict]) -> TypedDict:
        topic_list = defaultdict(list)
        for document in documents:
            topic_list[document['topic']].append(document['note'])
        
        return topic_list


class Note_taker(Module):

    module_name: str = "note_taker"

    ava: AvaModule
    storage: MongoDB

    def __init__(self):
        super().__init__(self.module_name)
        self.ava = AvaModule(self.module_name, "TODO", logging.DEBUG)
        self.storage = MongoDB(os.environ.get("MONGO_CONNECTION_STRING"), self.module_name)
        self.start()
        
    def start(self):
        while(True):
            module, action, slots = self.ava.waitForIntent()
            self.log.info("Module: %s, Action: %s, Slots: %s", module, action, slots)
            self.doAction(action, slots)

    def doAction(self, action: str, slots: TypedDict):
        if(action == 'add'):
            if(len(slots) == 0):
                self.ava.sayAndListen("")
            dictation = self.ava.sayAndListen("What do you want me to write down?")
            self.log.info("Got dictation: %s", dictation)
            
        elif (action == 'get'):
            self.ava.say("On it, getting your to-do list")
        elif (action == 'remove'):
            self.ava.say("What to-do do you want me to remove?")
        else:
            self.ava.say("Sorry this action is not yet implemented")

    def addNote(self, topic: str, msg: str) -> None:
        entity: Note = Note(topic, msg)
        self.storage.add(entity)

    def listAllNotes(self) -> TypedDict:
        return Note.groupByTopic(self.storage.getAll())
    
    def listNotesFromTopic(self, topic: str) -> List[str]:
        query: Note = Note(topic)
        return Note.groupByTopic(self.storage.get(query))[topic]

    def removeAll(self) -> None:
        query: Note = Note()
        self.storage.remove_many(query)
    
    def removeAllTopic(self, topic: str) -> None:
        query: Note = Note(topic)
        self.storage.remove_many(query)

    def removeSpecific(self, note: Note) -> None:
        self.storage.remove_one(note)

    def __del__(self):
        pass


if __name__ == "__main__":
    load_dotenv()
    note: Note_taker = Note_taker()
    # try:
    #     note.start()
    # except(TerminateModule):
    #     pass

    #del note

    # note.waitForIntent()
