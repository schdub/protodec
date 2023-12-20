PROTObuf2 DECompiler
====================
Decompiles protobuf (version 2) messages.

https://habr.com/ru/post/321790/

USAGE

    protodec [OPTIONS] path_to_file

OPTIONS

    --grab   - find and grab FileDescriptor data with meta information about
              .proto files from executable module .EXE or .DLL (.elf or .so).
    --schema - predict and print the schema of given raw message.
    --print  - print text representation of single message.
    --help   - this output.

Building
========

Regular binary:

```shell
qmake && make
```

Unit tests:

```shell
qmake "CONFIG += unittest" && make
```
