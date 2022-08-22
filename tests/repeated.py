#! /usr/bin/python3
import repeated_pb2
import sys

def store_to_file(msg, filename):
	f = open(filename, "wb")
	f.write(msg.SerializeToString())
	f.close()

def repeaded():
	rep = repeated_pb2.RepeatedPacked()
	rep.d.extend([3, 270, 86942])
	store_to_file(rep, "rep_pack.dat")

	rep_n = repeated_pb2.RepeatedNotPacked()
	rep_n.d.extend([3, 270, 86942])
	store_to_file(rep_n, "rep_not_pack.dat")

if __name__ == '__main__':
	repeaded()
