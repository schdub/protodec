#! /usr/bin/python3
import addressbook_pb2
import sys

def store_to_file(msg, filename):
	f = open(filename, "wb")
	f.write(msg.SerializeToString())
	f.close()

def address_book():
	address_book = addressbook_pb2.AddressBook()

	person = address_book.person.add()
	person.id = 1234
	person.name = "John Doe"
	person.email = "jdoe@example.com"
	phone = person.phone.add()
	phone.number = "555-4321"
	phone.type = addressbook_pb2.Person.HOME

	store_to_file(address_book, "addressbook.dat")

if __name__ == '__main__':
	address_book()
