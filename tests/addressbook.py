#! /usr/bin/python
import addressbook_pb2
import sys

address_book = addressbook_pb2.AddressBook()

person = address_book.person.add()
person.id = 1234
person.name = "John Doe"
person.email = "jdoe@example.com"
phone = person.phone.add()
phone.number = "555-4321"
phone.type = addressbook_pb2.Person.HOME

# Write the new address book back to disk.
f = open("addressbook.dat", "wb")
f.write(address_book.SerializeToString())
f.close()