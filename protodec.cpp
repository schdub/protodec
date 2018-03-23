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

#include "protoraw.hpp"

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
    data.resize(fileSize);
    file.read((char*)&data[0], fileSize);
}

// handling command options

struct CommandOptions {
    const char * mFilePath;
    bool         mPrint;
    bool         mSchema;
    bool         mShowUsage;

    void usage() {
        std::cout
            << "PROTObuf2 DECompiler v0.6 (c) Oleg V. Polivets, 2014-2017.\n"
            << "Decompiles protobuf (version 2) messages.\n"
            << "\n"
            << "protodec [OPTIONS] path_to_file\n"
            << "\n"
            << "OPTIONS:\n"
            << "--grab   - find and grab FileDescriptor data with meta information about\n"
            << "           .proto files from executable module .EXE or .DLL (.elf or .so).\n"
            << "--schema - preddict and print of the schema of given raw message.\n"
            << "--print  - print text reprisentation of single message.\n"
            << "--help   - this output.\n"
            << std::endl;
    }

    CommandOptions(int argc, char ** argv)
        : mFilePath(NULL)
        , mPrint(false)
        , mSchema(false)
        , mShowUsage(false)
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
