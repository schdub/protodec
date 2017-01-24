PROTObuf2 DECompiler
====================
Decompiles protobuf (version 2) messages.

USAGE

    protodec [OPTIONS] path_to_file

OPTIONS

    --grab   - find and grab FileDescriptor data with meta information about
              .proto files from executable module .EXE or .DLL (.elf or .so).
    --schema - preddict and print of the schema of given raw message.
    --print  - print text reprisentation of single message.
    --help   - this output.

Building
========

Regular binary:

    qmake && make

Unittests:

    qmake "CONFIG += unittest" && make
