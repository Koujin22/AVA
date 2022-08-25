from tkinter import E
from typing import List, TypeVar, TypedDict
from pymongo import MongoClient, database, collection
from abc import ABC, abstractmethod


class MongoEntity(ABC):

    @abstractmethod
    def get(self) -> TypedDict:
        pass

    @abstractmethod
    def query(self) -> TypedDict:
        pass
    

S = TypeVar('S', bound=MongoEntity)

class MongoDB():

    client: MongoClient
    db: database.Database
    table: collection.Collection

    def __init__(self, connection_string: str, collection_name: str):
        self.client = MongoClient(connection_string)
        self.db = self.client["note_taker"]
        self.table = self.db[collection_name]

    def add(self, entity: S) -> None:
        self.table.insert_one(entity.get())

    def get(self, entity: S) -> List[TypedDict]:
        documents = []
        cursor = self.table.find(entity.query())
        for document in cursor:
            documents.append(document)
        
        return documents

    def remove_many(self, entity: S) -> None:
        self.table.delete_many(entity.query())

    def remove_one(self, entity: S) -> None:
        self.table.delete_one(entity.query())

    def getAll(self) -> List[TypedDict]:
        documents = []
        cursor = self.table.find({})
        
        for document in cursor:
            documents.append(document)
        
        return documents

