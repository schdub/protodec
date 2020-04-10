// ///////////////////////////////////////////////////////////////////////// //
//                                                                           //
//   Copyright (C) 2014-2018 by Oleg Polivets                                //
//   jsbot@ya.ru                                                             //
//                                                                           //
//   This program is free software; you can redistribute it and/or modify    //
//   it under the terms of the GNU General Public License as published by    //
//   the Free Software Foundation; either version 2 of the License, or       //
//   (at your option) any later version.                                     //
//                                                                           //
//   This program is distributed in the hope that it will be useful,         //
//   but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           //
//   GNU General Public License for more details.                            //
//                                                                           //
// ///////////////////////////////////////////////////////////////////////// //

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <iterator>

#include "protoraw.hpp"
#include "version.h"

void readFile(
    std::vector<unsigned char> & data,
    const char * filename
) {
    data.clear();

    // open the file:
    std::streampos fileSize;
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) return;

    // get its size:
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
    fileSize += 2; // for 2 final zeroes
    data.resize(fileSize);
    file.read((char*)&data[0], fileSize);
}

// handling command options

struct CommandOptions {
    const char * mFilePath;
    bool         mPrint;
    bool         mSchema;
    bool         mShowUsage;
    bool         mJava;

    void usage() {
        std::cout
            << "PROTObuf2 DECompiler v" << _VERSION_ << " " << _PROD_COPYRIGHT_ "\n"
            << "Decompiles protobuf (version 2) messages.\n"
            << "\n"
            << "protodec [OPTIONS] path_to_file\n"
            << "\n"
            << "OPTIONS:\n"
            << "--grab   - find and grab FileDescriptor data with meta information about\n"
            << "           .proto files from executable module .EXE or .DLL (.elf or .so).\n"
            << "--schema - preddict and print of the schema of given raw message.\n"
            << "--print  - print text reprisentation of single message.\n"
            << "--java   - decrypt Java descriptor.\n"
            << "--help   - this output.\n"
            << std::endl;
    }

    CommandOptions(int argc, char ** argv)
        : mFilePath(NULL)
        , mPrint(false)
        , mSchema(false)
        , mShowUsage(false)
        , mJava(false)
    {
        for (int i = 1; i < argc; ++i) {
            if (!strcmp(argv[i], "--help")) {
                mShowUsage = true;
            } else if (!strcmp(argv[i], "--schema")) {
                mSchema = true;
            } else if (!strcmp(argv[i], "--print")) {
                mPrint = true;
            } else if (!strcmp(argv[i], "--grab")) {
                ++i;
                mFilePath = (i < argc ? argv[i] : NULL);
            } else if (!strcmp(argv[i], "--java")) {
                mJava = true;
            } else {
                mFilePath = argv[i];
            }
        }
        // if grab or print or schema command selected then not show usage
        mShowUsage = !(mFilePath || mPrint || mSchema);
    }

    ~CommandOptions() {
        if (mShowUsage) usage();
    }
};

inline
unsigned char xdigit(unsigned char ch) {
    if ('0' <= ch && ch <= '9') {
        return ch - '0';
    }
    else if ('a' <= ch && ch <= 'f') {
        return ch - 'a' + 10;
    }
    else if ('A' <= ch && ch <= 'F') {
        return ch - 'A' + 10;
    }
    else {
        std::cerr << "Unexpected hexadecimal digit " << ch << ".";
        exit(EXIT_FAILURE);
    }
}

// /////////////////////////////////////////////////////////////////// //

int main(int argc, char ** argv) {
    CommandOptions cmdOptions(argc, argv);
    if (cmdOptions.mFilePath) {
        std::vector<unsigned char> data;
        readFile(data, cmdOptions.mFilePath);
        if (data.empty()) {
            std::cerr << "ERROR: file '" << cmdOptions.mFilePath << "' "
                      << "is empty or not found."
                      << std::endl;
            return EXIT_FAILURE;
        }

        if (cmdOptions.mJava) {
            auto j = begin(data);
            for (auto i = begin(data); i != end(data); ++j, ++i) {
                if (*i != '\\') {
                    if (i != j) {
                        *j = *i;
                    }
                }
                else {
                    if (++i == end(data)) {
                        std::cerr << "ERROR: unescaped backslash at the end of a string." << std::endl;
                        return EXIT_FAILURE;
                    }

                    switch (*i) {
                        case 'n':
                            *j = '\n';
                            break;
                        case 't':
                            *j = '\t';
                            break;
                        case 'r':
                            *j = '\r';
                            break;
                        case '"':
                        case '\\':
                        case '\'':
                            *j = *i;
                            break;
                        case 'u': {
                            uint16_t val = 0;
                            for (size_t k = 0; k != 4; ++k) {
                                ++i;
                                if (i == end(data)) {
                                    std::cerr << "ERROR: not enough hexadecimal digits at the end of a string." << std::endl;
                                    exit(EXIT_FAILURE);
                                }
                                val <<= 4;
                                val |= xdigit(*i);
                            }
                            if (val > 0xff) {
                                std::cerr << "ERROR: unexpected escaped symbol at pos 0x" << std::hex << std::distance(begin(data), i) << " (0x" << val << std::dec << ")." << std::endl;
                                exit(EXIT_FAILURE);
                            }
                            *j = val;
                            break;
                        }
                        default:
                            std::cerr << "ERROR: unknown escape sequence: \\" << *i << "." << std::endl;
                            exit(EXIT_FAILURE);
                    }
                }
            }
            if (j != end(data)) {
                data.erase(j, end(data));
            }
        }

        data.push_back('\0');
        data.push_back('\0');

        const unsigned char *pB = &data[0], *pE = pB + data.size();
        if (!cmdOptions.mPrint && !cmdOptions.mSchema) {
            // trying to find and parse serialized_pb
            if (!Serialized_pb::grab(pB, pE)) {
                std::cerr << "ERROR: nothing is found." << std::endl;
                return EXIT_FAILURE;
            }
        } else {
            RawMessage msg;
            if (msg.parse(pB, pE)) {
                if (cmdOptions.mPrint)
                    msg.print(std::cout);
                else
                    Schema::print(msg, std::cout);
            }
            if (msg.isError()) {
                std::cerr << "ERROR: parsing failed " << msg.errorString() << "." << std::endl;
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}
