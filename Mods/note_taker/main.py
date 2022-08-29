from collections import defaultdict
from typing import List, TypedDict
from .._ava_module.AvaModule import AvaModule, CouldntConfirm, TerminateModule
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
        super().__init__(self.module_name, logging.INFO)
        self.ava = AvaModule(self.module_name, "TODO", logging.INFO)
        self.storage = MongoDB(os.environ.get("MONGO_CONNECTION_STRING"), self.module_name)
        
    def start(self):
        while(True):
            module, action, slots = self.ava.waitForIntent()
            self.log.info("Module: %s, Action: %s, Slots: %s", module, action, slots)
            self.doAction(action, slots)

    def doAction(self, action: str, slots: TypedDict):
        if(action == 'add'):
            self.add(slots)
        elif (action == 'get'):
            self.get(slots)
        elif (action == 'remove_all'):
            self.removeAll(slots)
        elif (action == 'remove'):
            self.remove(slots)
        else:
            self.ava.say("Sorry this action is not yet implemented")
        self.ava.finish()

    def remove(self, slots: TypedDict):
        position = int(slots.get('numero')) if slots.get('numero') is not None else int(slots['posicion'][0])
        category = slots['topicos']
        try:
            noteToDelete = self.__listNotesFromTopic(category)[position-1]
            self.__removeSpecific(Note(category, noteToDelete))
            self.ava.say("Done!")
        except IndexError:
            self.ava.say(("Im sorry, but in %s category theres no note number %s"% (category, position)))

    def removeAll(self, slots: TypedDict):
        try:
            if(len(slots) == 0):
                confirmation = self.ava.confirmation("Are you sure you want to remove all notes?")
            else:
                confirmation = self.ava.confirmation("Are you sure you want to remove all notes from %s?"%slots["topicos"])
            if(confirmation):
                if(len(slots) == 0):
                    self.ava.say("Got it, removing all notes")
                    self.__removeAll()
                else:
                    self.ava.say("Got it, removing all notes from %s"%slots["topicos"])
                    self.__removeAllTopic(slots["topicos"])
            else:
                self.ava.say("Copy that, cancelling deletion of tasks.")
        except(CouldntConfirm):
            self.ava.say("Sorry, I didnt understand that, but I will cancel the removal of to-do")
       

    def get(self, slots: TypedDict):
        if(len(slots) == 0):
            self.ava.say("On it, getting your to-do list")
            notes: TypedDict = self.__listAllNotes()
            for category in notes.keys():
                self.ava.say("Your notes on %s are"%category)
                for todo in notes[category]:
                    self.ava.say(todo, lang="es-us")
            self.ava.say("Thats all.")
        else:
            self.ava.say("On it, getting your to-do list on %s"%slots["topicos"])
            notes: List[str] = self.__listNotesFromTopic(slots["topicos"])
            self.ava.say("Your notes on %s are"%slots["topicos"])
            for todo in notes:
                self.ava.say(todo, lang="es-us")
            self.ava.say("Thats all.")

    def add(self, slots: TypedDict):
        if(len(slots) == 0):
            topic = self.ava.sayAndListen("Sure! To what topic do you want me to add it?", listen_time=2)
            if(topic not in ["ava", "escuela", "personal", "trabajo"]):
                self.ava.say("Im sorry, but %s is not in the accepted topics."%topic)
                self.ava.finish()
                return                
        else:
            topic = slots["topicos"]
        dictation = self.ava.sayAndListen("What do you want me to write down?")
        self.log.info("Got dictation: %s", dictation)
        self.ava.say("Adding")
        self.ava.say(dictation, lang="es-us")
        self.ava.say("to %s category"%topic)
        self.__addNote(topic, dictation)

    def __addNote(self, topic: str, msg: str) -> None:
        entity: Note = Note(topic, msg)
        self.storage.add(entity)

    def __listAllNotes(self) -> TypedDict:
        return Note.groupByTopic(self.storage.getAll())
    
    def __listNotesFromTopic(self, topic: str) -> List[str]:
        query: Note = Note(topic)
        return Note.groupByTopic(self.storage.get(query))[topic]

    def __removeAll(self) -> None:
        query: Note = Note()
        self.storage.remove_many(query)
    
    def __removeAllTopic(self, topic: str) -> None:
        query: Note = Note(topic)
        self.storage.remove_many(query)

    def __removeSpecific(self, note: Note) -> None:
        self.storage.remove_one(note)

    def __del__(self):
        pass


if __name__ == "__main__":
    load_dotenv()
    note: Note_taker = Note_taker()
    try:
        note.start()
    except(TerminateModule):
        pass

    del note

