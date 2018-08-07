#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <gtest/gtest.h>
#include "protoraw.hpp"

static void readFile(
    std::vector<unsigned char> & data,
    const char * filename
) {
    data.clear();
    std::streampos fileSize;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    data.resize(fileSize);
    file.read((char*)&data[0], fileSize);
}

///*

TEST(RawMessage, asciiString) {
    std::string s;
    s = "1234566789adfsdfsdfsdfSZZZZ ds ?? 1";
    ASSERT_TRUE(RawMessage::itsAsciiString(s.c_str(), s.c_str() + s.length())); 
    s = "12345667\u000589adf\u0001sdfsdfsdfSZZZZ ds ?? 1";
    ASSERT_FALSE(RawMessage::itsAsciiString(s.c_str(), s.c_str() + s.length())); 
}

TEST(RawMessage, varIntRead) {
    int64_t value = 0;
    unsigned char data[] = { 0xbd, 0x01 };
    const unsigned char *ptr = RawMessage::readVarint(data, data + 2, value);
    ASSERT_EQ(value, 189);
    ASSERT_EQ(ptr, data+2);
}

TEST(RawMessage, varIntWrite) {
    unsigned char data[2], *ptr = NULL;

    // single zero value
    data[0] = 0xff; data[1] = 0xff;
    ptr = RawMessage::writeVarint(0, data, data + 2);
    ASSERT_EQ(data[0], 0x00);
    ASSERT_EQ(data[1], 0xff);
    ASSERT_EQ(ptr, data+1);

    // 189 -> bd 01
    data[0] = 0x00; data[1] = 0x00;
    ptr = RawMessage::writeVarint(189, data, data + 2);
    ASSERT_EQ(data[0], 0xbd);
    ASSERT_EQ(data[1], 0x01);
    ASSERT_EQ(ptr, data+2);

    // not enough bytes to store
    data[0] = 0x00; data[1] = 0x00;
    ptr = RawMessage::writeVarint(189, data, data + 1);
    ASSERT_EQ(data[0], 0xbd);
    ASSERT_EQ(data[1], 0x00);
    ASSERT_EQ(ptr, data+1);
}

TEST(RawMessage, parsing) {
    {
    unsigned char data[] = {
        0x0a, 0x04, '0', '1', '2', '3'
    };
    size_t len = (sizeof(data)/sizeof(*data));
    RawMessage msg;
    msg.parse(data, data + len);
    ASSERT_EQ(msg.items().size(), 1);
    ASSERT_EQ(msg.items().count(1), 1);
    }
    {
    unsigned char data[] = {
        0x0a, 0x05, '0', '1', '2', '3', '4',
        0x0a, 0x04, 'a', 'b', 'c', 'd',
        0x0a, 0x03, 'X', 'Y', 'Z'
    };
    size_t len = (sizeof(data)/sizeof(*data));
    RawMessage msg;
    msg.parse(data, data + len);
    ASSERT_EQ(msg.items().size(), 1);
    ASSERT_EQ(msg[1]->asStringMap(1), "01234");
    ASSERT_EQ(msg[1]->asStringMap(2), "abcd");
    ASSERT_EQ(msg[1]->asStringMap(3), "XYZ");
    }
}

TEST(RawMessage, sizeInBytes) {
    {
    unsigned char data[] = {
        0x0a, 0x04, '0', '1', '2', '3'
    };
    size_t len  = (sizeof(data)/sizeof(*data));
    RawMessage msg;
    msg.parse(data, data + len);
    ASSERT_EQ(msg.getSizeInBytes(msg.items()), len);
    }
    {
    unsigned char data[] = {
        0x0a, 0x05, '0', '1', '2', '3', '4',
        0x0a, 0x04, 'a', 'b', 'c', 'd',
        0x0a, 0x03, 'X', 'Y', 'Z'
    };
    size_t len = (sizeof(data)/sizeof(*data));
    RawMessage msg;
    msg.parse(data, data + len);
    ASSERT_EQ(msg.getSizeInBytes(msg.items()), len);
    }
}

TEST(RawMessage, printing) {
    unsigned char d1[] = {
        0x0a, 0x05, '0', '1', '2', '3', '4',
        0x0a, 0x04, 'a', 'b', 'c', 'd',
        0x0a, 0x03, 'X', 'Y', 'Z'
    };
    unsigned char d2[] = {
        0x22,            // tag (field number 4, wire type 2)
        0x06,            // payload size (6 bytes)
        0x03,            // first element (varint 3)
        0x8E, 0x02,      // second element (varint 270)
        0x9E, 0xA7, 0x05 // third element (varint 86942)
    };
    unsigned char*  data[2] = { d1, d2};
    unsigned     lengths[2] = { sizeof(d1), sizeof(d2)};
    std::string expected[2] = {
      "1 [\n"
      "\t1: \"01234\"\n"
      "\t2: \"abcd\"\n"
      "\t3: \"XYZ\"\n"
      "]\n",
      "4: \"\x3\\142\x2\\158\\167\\005\"\n"
    };
    for (unsigned i = 0; i < sizeof(data)/sizeof(*data); ++i) {
      size_t len = lengths[i];
      std::stringstream ss;
      RawMessage msg;
      msg.parse(data[i], data[i] + len);
      msg.print(ss);
      const std::string & actual = ss.str();
      ASSERT_EQ(actual, expected[i]);
    }
}

//*/

TEST(Schema, print) {
    {
    unsigned char data[] = {
        0x0a, 0x05, '0', '1', '2', '3', '4',
        0x0a, 0x04, 'a', 'b', 'c', 'd',
        0x0a, 0x03, 'X', 'Y', 'Z'
    };
    std::string expected = "package ProtodecMessages;\n"
                           "\n"
                           "message MSG1 {\n"
                           "\trepeated string fld1 = 1;\n"
                           "}\n";
    size_t len = (sizeof(data)/sizeof(*data));
    RawMessage msg;
    std::stringstream ss;
    ASSERT_TRUE(msg.parse(data, data + len));
    Schema::print(msg, ss);
    ASSERT_EQ(ss.str(), expected);
    }{
    unsigned char data[] = {
        0x0a, 0x05, '0', '1', '2', '3', '4'
    };
    std::string expected = "package ProtodecMessages;\n"
                           "\n"
                           "message MSG1 {\n"
                           "\trequired string fld1 = 1;\n"
                           "}\n";
    size_t len = (sizeof(data)/sizeof(*data));
    RawMessage msg;
    std::stringstream ss;
    ASSERT_TRUE(msg.parse(data, data + len));
    Schema::print(msg, ss);
    ASSERT_EQ(ss.str(), expected);
    }{
    int rc = 0;
    std::string expected =
        "package ProtodecMessages;\n"
        "\n"
        "message MSG1 {\n"
        "\trequired string fld1 = 1;\n"
        "\trequired int64 fld2 = 2;\n"
        "}\n"
        "\n"
        "message MSG2 {\n"
        "\trequired string fld1 = 1;\n"
        "\trequired int64 fld2 = 2;\n"
        "\trequired string fld3 = 3;\n"
        "\trequired MSG1 fld4 = 4;\n"
        "}\n"
        "\n"
        "message MSG3 {\n"
        "\trequired MSG2 fld1 = 1;\n"
        "}\n";

    const char addressbook_dat_path[] = "addressbook.dat";
    ::remove(addressbook_dat_path);
    rc = system("protoc --python_out=. tests/addressbook.proto");
    ASSERT_EQ(rc, 0);
    rc = system("python tests/addressbook.py");
    ASSERT_EQ(rc, 0);

    std::vector<unsigned char> addressbook_raw;
    readFile(addressbook_raw, addressbook_dat_path);
    ASSERT_TRUE( addressbook_raw.size() > 0 );

    RawMessage msg;
    bool parsed = msg.parse(addressbook_raw.data(), addressbook_raw.data() + addressbook_raw.size());
    ASSERT_TRUE(parsed);

    std::stringstream ss;
    Schema::print(msg, ss);
    ASSERT_EQ(ss.str(), expected);

    ::remove(addressbook_dat_path);
    }
}

TEST(Serialized_pb, find) {
    char data[] = "BEGINOFGARBIGEGARBIGEGARBIGEGARBIGEGARBIGEGARBIGE"
                  "GARBIGEGARBIGEGARBIGEGARBIGEGARBIGEGARBIGEGARBIGE"
                  "\n\x11\x61\x64\x64ressbook.proto\x12\x08tutorial\"\xda\x01\n\x06Person\x12\x0c\n\x04name\x18\x01 \x02(\t\x12\n\n\x02id\x18\x02 \x02(\x05\x12\r\n\x05\x65mail\x18\x03 \x01(\t\x12+\n\x05phone\x18\x04 \x03(\x0b\x32\x1c.tutorial.Person.PhoneNumber\x1aM\n\x0bPhoneNumber\x12\x0e\n\x06number\x18\x01 \x02(\t\x12.\n\x04type\x18\x02 \x01(\x0e\x32\x1a.tutorial.Person.PhoneType:\x04HOME\"+\n\tPhoneType\x12\n\n\x06MOBILE\x10\x00\x12\x08\n\x04HOME\x10\x01\x12\x08\n\x04WORK\x10\x02\"/\n\x0b\x41\x64\x64ressBook\x12 \n\x06person\x18\x01 \x03(\x0b\x32\x10.tutorial.Person"
                  "\0ENDOFGARBIGEGARBIGEGARBIGEGARBIGEGARBIGEGARBIGE"
                  "GARBIGEGARBIGEGARBIGEGARBIGEGARBIGEGARBIGEGARBIGE";
    const unsigned char *pB = (unsigned char*) data, *pE = pB + (sizeof(data)/sizeof(*data)), *ptr;
    ptr = Serialized_pb::findSerializedPB(pB, pE);
    ASSERT_TRUE(ptr != NULL);
    ASSERT_EQ(strncmp((char*)pE+1, "ENDOFGARBIGE", 12), 0);
    ASSERT_TRUE(RawMessage::isValidMessage(ptr, pE));
}

TEST(Serialized_pb, grab) {
    {
    RawMessage msg;
    char data[] = "\n\x11\x61\x64\x64ressbook.proto\x12\x08tutorial\"\xda\x01\n\x06Person\x12\x0c\n\x04name\x18\x01 \x02(\t\x12\n\n\x02id\x18\x02 \x02(\x05\x12\r\n\x05\x65mail\x18\x03 \x01(\t\x12+\n\x05phone\x18\x04 \x03(\x0b\x32\x1c.tutorial.Person.PhoneNumber\x1aM\n\x0bPhoneNumber\x12\x0e\n\x06number\x18\x01 \x02(\t\x12.\n\x04type\x18\x02 \x01(\x0e\x32\x1a.tutorial.Person.PhoneType:\x04HOME\"+\n\tPhoneType\x12\n\n\x06MOBILE\x10\x00\x12\x08\n\x04HOME\x10\x01\x12\x08\n\x04WORK\x10\x02\"/\n\x0b\x41\x64\x64ressBook\x12 \n\x06person\x18\x01 \x03(\x0b\x32\x10.tutorial.Person";
    size_t len = (sizeof(data)/sizeof(*data));
    ASSERT_TRUE(msg.parse((unsigned char*) data, (unsigned char*) data + len));

    std::stringstream ss;
    std::string expected = "package tutorial;\n"
                           "message Person {\n"
                           "\tenum PhoneType {\n"
                           "\t\tMOBILE = 0;\n"
                           "\t\tHOME = 1;\n"
                           "\t\tWORK = 2;\n"
                           "\t}\n"
                           "\tmessage PhoneNumber {\n"
                           "\t\trequired string number = 1;\n"
                           "\t\toptional .tutorial.Person.PhoneType type = 2 [default = HOME];\n"
                           "\t}\n"
                           "\trequired string name = 1;\n"
                           "\trequired int32 id = 2;\n"
                           "\toptional string email = 3;\n"
                           "\trepeated .tutorial.Person.PhoneNumber phone = 4;\n"
                           "}\n"
                           "message AddressBook {\n"
                           "\trepeated .tutorial.Person person = 1;\n"
                           "}\n";
    Serialized_pb::printMessagesFromSerialized(msg, ss);
    ASSERT_EQ(ss.str(), expected);
    }{
        unsigned char data[] = {
            0x0a, 0x07, 't', '.', 'p', 'r', 'o', 't', 'o',
            0x2a, 0x12, 0x0a, 0x06, 'D', 'o', 'm', 'a', 'i', 'n',
            0x12, 0x08, 0x0a, 0x04, 'U', 'S', 'E', 'R',
            0x10, 0x01
        };
        std::string expected = "enum Domain {\n"
                               "\tUSER = 1;\n"
                               "}\n";
        size_t len = (sizeof(data)/sizeof(*data));
        RawMessage msg;
        ASSERT_TRUE(msg.parse((unsigned char*) data, (unsigned char*) data + len));
        std::stringstream ss;
        Serialized_pb::printMessagesFromSerialized(msg, ss, true);
        const std::string & actual = ss.str();
        ASSERT_EQ(actual, expected);
    }
}

int main(int argc, char ** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
