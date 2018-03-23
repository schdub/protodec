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

#pragma once

#include <iostream>
#include <map>
#include <stack>
#include <memory>
#include <cassert>
#include <sstream>
#include <algorithm>
#include <stdexcept>

class RawMessage {
public:

    class Variant;
    typedef std::shared_ptr<Variant> VariantPtr;
    typedef std::map<unsigned, VariantPtr> KeyValueMap;
    friend std::ostream& operator<<(std::ostream & os, const Variant & var);

    static const VariantPtr At(const KeyValueMap & map, unsigned k) {
        KeyValueMap::const_iterator it = map.find(k);
        if (it != map.end()) {
            return it->second;
        }
        std::stringstream ss;
        ss << "ERROR: Key '" << k << "' not found.";
        throw std::logic_error(ss.str());
    }

    VariantPtr rootItem() const {
        assert( mRoot );
        return mRoot;
    }
    KeyValueMap items() const {
        assert(mRoot && !mRoot->asMap().empty() );
        return mRoot->asMap();
    }

    const VariantPtr operator[] (unsigned idx) const {
        assert( !mRoot->asMap().empty() );
        KeyValueMap::const_iterator it = mRoot->asMap().find(idx);
        assert(it != mRoot->asMap().end());
        return it->second;
    }

// ///////////////////////////////////////////////////////////////////////// //

class Variant {
public:
    // protobuf data types
    enum PROTO2TYPES {
        proto2Varint = 0,
        proto2Double = 1,
        proto2Buffer = 2,
        proto2Float  = 5,
    };

    enum TYPE {
        vtEmpty,
        vtString,
        vtInteger,
        vtFloat,
        vtDouble,
        vtRepeated,
        vtNode
    };

    explicit Variant(TYPE dataType = vtEmpty)
        : mDataType(dataType)
        , mSubNodesSize(0)
        , mIndex(0)
        , mNumber(0)
    {
    }

    explicit Variant(const char* string, unsigned lenght)
        : mDataType(vtString)
        , mSubNodesSize(0)
        , mString(string, lenght)
        , mIndex(0)
        , mNumber(0)
    {
    }

    explicit Variant(int64_t value)
        : mDataType(vtInteger)
        , mSubNodesSize(0)
        , mIndex(0)
        , mNumber(0)
    {
        mData.i = value;
    }

    explicit Variant(double value)
        : mDataType(vtDouble)
        , mSubNodesSize(0)
        , mIndex(0)
        , mNumber(0)
    {
        mData.d = value;
    }

    explicit Variant(float value)
        : mDataType(vtFloat)
        , mSubNodesSize(0)
        , mIndex(0)
        , mNumber(0)
    {
        mData.f = value;
    }

    ~Variant() {}

    bool hasField(unsigned index) const {
        assert(isMap() || isRepeated());
        KeyValueMap::const_iterator it = mNodes.find(index);
        return (it != mNodes.end());
    }

    int64_t getFieldValue() const {
        int type = 0;

        switch (mDataType) {
        case vtInteger: type = proto2Varint; break;
        case vtDouble:  type = proto2Double; break;
        case vtString:  type = proto2Buffer; break;
        case vtFloat:   type = proto2Float;  break;
        default: assert(!"unknown type");
        }

        int64_t retValue = mIndex << 3;
        retValue |= type;

        return retValue;
    }

    void setIndex(unsigned value) {
        mIndex = value;
    }
    unsigned index() const {
        return mIndex;
    }

    void setGlobalId(unsigned value) {
        assert(value > 0);
        mNumber = value;
    }
    unsigned globalId() const {
        assert(mNumber > 0);
        return mNumber;
    }

    void setSubNodesSize(int64_t value) {
        mSubNodesSize = value;
    }
    int64_t subNodesSize() const {
        assert(isMap());
        return mSubNodesSize;
    }

    VariantPtr operator[] (unsigned idx) {
        assert(isMap() || isRepeated());
        return mNodes[idx];
    }

    static VariantPtr make() {
        return VariantPtr(new Variant());
    }
    static VariantPtr makeMap() {
        return VariantPtr(new Variant(vtNode));
    }
    static VariantPtr makeRepeated() {
        return VariantPtr(new Variant(vtRepeated));
    }
    static VariantPtr make(const char * data, unsigned lenght) {
        return VariantPtr(new Variant(data, lenght));
    }
    template <class T>
    static VariantPtr make(T value) {
        return VariantPtr(new Variant(value));
    }

    bool isRepeated() const {
        return mDataType == vtRepeated;
    }

    bool isString() const {
        return mDataType == vtString;
    }
    const std::string & asString() const {
        assert(isString());
        return mString;
    }
    std::string & asString() {
//        assert(isString());
        return mString;
    }

    bool isInt() const {
        return mDataType == vtInteger;
    }
    int64_t asInt() const {
        assert(isInt());
        return mData.i;
    }
    int64_t& asInt() {
        assert(isInt());
        return mData.i;
    }

    bool isFloat() const {
        return mDataType == vtFloat;
    }
    float asFloat() const {
        assert(isFloat());
        return mData.f;
    }
    float& asFloat() {
        assert(isFloat());
        return mData.f;
    }

    bool isDouble() const {
        return mDataType == vtDouble;
    }
    double asDouble() const {
        assert(isDouble());
        return mData.d;
    }
    double& asDouble() {
        assert(isDouble());
        return mData.d;
    }

    bool isMap() const {
        return (mDataType == vtNode);
    }
    const KeyValueMap & asMap() const {
        assert(isMap() || isRepeated());
        return mNodes;
    }
    KeyValueMap & asMap() {
        assert(isMap() || isRepeated());
        return mNodes;
    }
    const std::string & asStringMap(unsigned idx) const {
        assert(isMap() || isRepeated());
        KeyValueMap::const_iterator it = mNodes.find(idx);
        assert(it != mNodes.end());
        return it->second->asString();
    }
    std::string & asStringMap(unsigned idx) {
        assert(isMap() || isRepeated());
        return mNodes[idx]->asString();
    }

    std::string dataType() const {
        switch (mDataType) {
        case vtInteger: return "int64";
        case vtDouble:  return "double";
        case vtString:  return "string";
        case vtFloat:   return "float";
        default: {
            std::stringstream ss;
            ss << "MSG" << globalId();
            return ss.str();
        }}
    }

private:
    TYPE mDataType;
    int64_t mSubNodesSize;
    union {              // wrap access to int, float and double data types
        int64_t i;
        float f;
        double d;
    } mData;
    std::string mString; // data for vtString and content of message for vtMap
    KeyValueMap mNodes;  // subnodes for vtRepeated or vtMap data type
    unsigned mIndex;     // index of this field in root message
    unsigned mNumber;    // number of message in global instance
};

// ///////////////////////////////////////////////////////////////////////// //

    template<class T>
    static bool itsAsciiString(const T * beg, const T * end) {
        for (; beg < end; ++beg) {
            if (!*beg || !isprint(*beg)) {
                return false;
            }
        }
        return true;
    }

    template<class T>
    static const T * readVarint(const T * beg, const T * end, int64_t & value) {
        int64_t temp = 0, cnt = 0;
        for (value = 0; beg < end && cnt < 16; ++beg, ++cnt) {
            //assert(cnt < 16);
            temp += (*beg & 0x7f) << (cnt * 7);
            if (!(*beg & 0x80)) {
                ++beg;
                break;
            }
        }
#if DEBUG
//        std::cerr << "readVarint() " << temp << std::endl;
#endif
        value = temp;
        return beg;
    }

    template<class T>
    static T * writeVarint(int64_t value, T * beg, T * end) {
        while (beg < end) {
            if (value > 0x7f) {
                *beg++ = (0x80 | (value & 0x7f));
                value >>= 7;
            } else {
                *beg++ = (value & 0x7f);
                return beg;
            }
        }
        assert(value < 0x7f);
        return beg;
    }

    template<class P, class T>
    static const P * readValue(const P * beg, const P * end, T & value) {
        assert(beg + sizeof(T) <= end);
        value = *((const T *)beg);
#if DEBUG
        std::cerr << __FUNCTION__ << " " << value << std::endl;
#endif
        return beg + sizeof(T);
    }

    template<class T>
    static bool isValidMessage(const T * p, const T * e) {
#if DEBUG
        const T * start = p;
        std::cerr << __FUNCTION__
                  << " start=0x" << std::hex << (std::ptrdiff_t) (p-0)
                  <<   " end=0x" << std::hex << (std::ptrdiff_t) (e-p)
                  << std::endl;
#endif
        int prevIdx = -1;
        int64_t intValue;
        for (;;) {
            if (p < e) {
                // read field and data type
                p = readVarint(p, e, intValue);
                if (intValue == 0) {
#if DEBUG
                    std::cerr << "ZERO offset 0x"
                              << std::hex << (p - start)
                              << std::endl;
#endif
                    continue;
                }

                int type = (intValue  & 7);
                int idx  = (intValue >> 3);

#if DEBUG
                std::cerr << idx << ":" <<  type << std::endl;
#endif

                if (idx < prevIdx) {
#if DEBUG
                    std::cerr << "WRONG index" << std::endl
                              << "offset 0x"   << std::hex << (p - start)
                              << std::endl;
#endif
                    break;
                } else {
                    prevIdx = idx;
                }

                if (p >= e) {
#if DEBUG
                    std::cerr << "offset 0x" << std::hex << (p - start)
                              << std::endl;
#endif
                    break;
                }

                // check data type
                if (type == 0 || type == 2) {
                    p = readVarint(p, e, intValue);
                } else if (type == 5) {
                    p += sizeof(float);
                } else if (type == 1) {
                    p += sizeof(double);
                } else {
#if DEBUG
                    std::cerr << "unknown data type" << std::endl
                              << "offset 0x" << std::hex << (p - start) << std::endl
                              << "type = " << type << std::endl
                              << "idx  = " << idx  << std::endl;
#endif
                    //assert(!"unknown data type");
                    break;
                }

                // sub message or buffer contents
                if (type == 2) {
                    p += intValue;
                }
            }
            if (p == e) {
                return true;
            } else if (p > e) {
#if DEBUG
                std::cerr << "(p > e) offset 0x"
                          << std::hex << (std::ptrdiff_t) (p-0)
                          << std::endl;
#endif
                break;
            }
        }
        return false;
    }

    void mapInsert(unsigned idx, KeyValueMap & map, VariantPtr pVariant) {
        // set index
        pVariant->setIndex(idx);

        // ordinary addition
        if (map.count(idx) == 0) {
            map[idx] = pVariant;
            return;
        }

        VariantPtr pBase = map[idx];
        if (!pBase->isRepeated()) {
            VariantPtr ptr = pBase;
            pBase = Variant::makeRepeated();
            pBase->asMap()[1] = ptr;
            map[idx] = pBase;
        }

        pBase->asMap()[pBase->asMap().size()+1] = pVariant;
    }

    bool parse(
        const unsigned char * start,
        const unsigned char * e
    ) {
        mError = "data corrupted";
#if DEBUG
        std::cerr << __FUNCTION__
                  << " start=0x" << std::hex << (std::ptrdiff_t) (start-0)
                  <<   " end=0x" << std::hex << (std::ptrdiff_t) (e-0)
                  << std::endl;
#endif
        if (start >= e) {
            return false;
        }

        const unsigned char * p = start;
        std::stack< const unsigned char* > tails;
        std::stack< KeyValueMap* > messages;
        KeyValueMap * currentMap = 0;

        mRoot.reset();
        mRoot = Variant::makeMap();
        tails.push(e);
        messages.push(&mRoot->asMap());

        while (!tails.empty()) {
            e = tails.top();
            currentMap = messages.top();
            int64_t intValue;
            double dblValue;
            float fltValue;
            for (;;) {
                if (p < e) {
                    // read field and data type
                    p = readVarint(p, e, intValue);

                    if (intValue == 0) {
                        continue;
                    }

                    int type = (intValue  & 7);
                    int idx  = (intValue >> 3);
#if DEBUG
                    std::cerr << type << ":" << idx << std::endl;
#endif
                    if (p>=e) {
                        std::stringstream ss;
                        ss << "offset 0x" << std::hex << (p - start);
                        mError = ss.str();
#if DEBUG
                        std::cerr << mError << std::endl;
#endif
                        return false;
                    }

                    // check data type
                    if (type == 0 || type == 2) {
                        p = readVarint(p, e, intValue);
                    } else if (type == 5) {
                        p = readValue(p, e, fltValue);
                    } else if (type == 1) {
                        p = readValue(p, e, dblValue);
                    } else {
                        std::stringstream ss;
                        ss << "unknown data type" << std::endl
                           << "offset 0x" << std::hex << (p - start) << std::endl
                           << "type = " << type << std::endl
                           << "idx  = " << idx;
                        mError = ss.str();
#if DEBUG
                        std::cerr << mError << std::endl;
#endif
                        //assert(!"unknown data type");
                        return false;
                    }

                    // read contents
                    if (type == 5 || type == 1 || type == 0) {
                        VariantPtr newNode(
                            type == 0 ? Variant::make(intValue) :
                            type == 1 ? Variant::make(dblValue) :
                                        Variant::make(fltValue));
                        mapInsert(idx, *currentMap, newNode);
                    } else if (type == 2) {
                        assert(p+intValue <= e);
                        if (p+intValue > e) {
                            return false;
                        }

                        bool isString = itsAsciiString(p, p + intValue);
                        if (isString || !isValidMessage(p, p + intValue)) {
                            // ascii string or buffer
                            VariantPtr newString(Variant::make((char*)p, intValue));
#if DEBUG
                            std::cerr << idx << ": " << newString->asString().c_str() << std::endl;
#endif
                            mapInsert(idx, *currentMap, newString);
                            p += intValue;
                        } else {
                            // submessage
                            VariantPtr newNode(Variant::makeMap());
#if DEBUG
                            std::cerr << idx << ": SUBMESSAGE" << (tails.size()+1)
                                      << std::endl
                                      << "offset 0x" << std::hex << (p - start)
                                      << " end 0x"   << std::hex << (p + intValue - start)
                                      << std::endl;
#endif
                            tails.push(p + intValue);
                            messages.push(&(newNode->asMap()));
                            mapInsert(idx, *currentMap, newNode);
                            break;
                        }
                    }
                }

                if (p >= e) {
#if DEBUG
                    std::cerr << "~ SUBMESSAGE"
                              << tails.size()
                              << " ===================================="
                              << std::endl
                              << "offset 0x" << std::hex << (p - start)
                              << " end 0x"   << std::hex << (e - start)
                              << std::endl;
#endif
                    tails.pop();
                    messages.pop();
                    break;
                }
            }
        }
        mError.clear();
        return true;
    }

    // /////////////////////////////////////////////////////////////////// //

    static void printMessageInternal(
        const RawMessage::KeyValueMap & map,
        std::ostream & os,
        int indent = 0
    ) {
        for (RawMessage::KeyValueMap::const_iterator it = map.begin(); it != map.end(); ++it) {
            const RawMessage::VariantPtr & var = it->second;
            if (var->isMap()) {
                assert(!var->asMap().empty());
                for (int i = 0; i < indent; ++i) os << '\t';
                os << it->first << " {\n";
                printMessageInternal(var->asMap(), os, indent+1);
                for (int i = 0; i < indent; ++i) os << '\t';
                os << "}\n";
            } else if (var->isRepeated()) {
                assert(!var->asMap().empty());
                for (int i = 0; i < indent; ++i) os << '\t';
                os << it->first << " [\n";
                printMessageInternal(var->asMap(), os, indent+1);
                for (int i = 0; i < indent; ++i) os << '\t';
                os << "]\n";
            } else {
                for (int i = 0; i < indent; ++i) os << '\t';
                os << it->first << ": " << *var << std::endl;
            }
        }
    }

    void print(std::ostream & os, int indent = 0) const {
        printMessageInternal(items(), os, indent);
    }

    // /////////////////////////////////////////////////////////////////// //

    size_t bytes7bit(int64_t value) {
        for (int i = 1, t=7; i <= 8; ++i, t*=7)
            if (value < (1<<t)) return i;
        assert(!"this shouldn't happen");
        return 0;
    }

    // calculate size in bytes of given message
    size_t getSizeInBytes(const KeyValueMap & map) {
        size_t retValue = 0, t;
        for (KeyValueMap::const_iterator it = map.begin(); it != map.end(); ++it) {
            const VariantPtr & var = it->second;
            if (var->isMap()) {
                assert(!var->asMap().empty());
                t  = getSizeInBytes(var->asMap());
                t += bytes7bit(t);
                t += bytes7bit(var->getFieldValue());
                retValue += t;
            } else if (var->isRepeated()) {
                assert(!var->asMap().empty());
                t  = getSizeInBytes(var->asMap());
                retValue += t;
            } else if (var->isInt()) {
                t  = bytes7bit(var->asInt());
                t += bytes7bit(var->getFieldValue());
                retValue += t;
            } else if (var->isFloat()) {
                t  = sizeof(float);
                t += bytes7bit(var->getFieldValue());
                retValue += t;
            } else if (var->isDouble()) {
                t  = sizeof(double);
                t += bytes7bit(var->getFieldValue());
                retValue += t;
            } else if (var->isString()) {
                t  = var->asString().length();
                t += bytes7bit(t);
                t += bytes7bit(var->getFieldValue());
                retValue += t;
            } else {
                assert(!"This shouldn't happen.");
            }
        }
        return retValue;
    }

public:

    bool isError() const {
        return !mError.empty();
    }
    const std::string & errorString() const {
        return mError;
    }

private:
    VariantPtr mRoot;
    std::string mError;
}; // RawMessage

// /////////////////////////////////////////////////////////////////// //

class Serialized_pb {

    static void printField(
        const RawMessage::KeyValueMap & vit,
        std::ostream & os,
        int indent = 0
    ) {
        // data type
        static std::string types[] = {
            "double",
            "float",
            "int64",
            "uint64",
            "int32",
            "fixed64",
            "fixed32",
            "bool",
            "string",
            "group",
            "message",
            "bytes",
            "uint32",
            "enum",
            "sfixed32",
            "sfixed64",
            "sint32",
            "sint64"
        };
        static unsigned typesCount = sizeof(types) / sizeof(*types);

        unsigned dataType = RawMessage::At(vit, 5)->asInt();
        assert( dataType > 0 && dataType < typesCount );
        const bool isComplexType = (dataType == 11 || dataType == 14);
        const std::string & strDataType = isComplexType
            ? RawMessage::At(vit,6)->asString() : types[dataType-1];

        // label of current field
        static std::string labels[] = {
            "optional",
            "required",
            "repeated"
        };
        static unsigned labelsCount = sizeof(labels) / sizeof(*labels);

        int label = RawMessage::At(vit,4)->asInt()-1;
        assert(label < labelsCount);
        const std::string & strLabel = labels[label];

        std::string strDefault;
        if (vit.count(7)) {
            strDefault.append(" [default = ");
                        strDefault.append(RawMessage::At(vit,7)->asString());
                        strDefault.append("]");
        }

        for (int i = 0; i < indent; ++i) os << '\t';
        os << strLabel.c_str()              << " "
           << strDataType.c_str()           << " "
           << RawMessage::At(vit,1)->asString().c_str() << " = "
           << RawMessage::At(vit,3)->asInt()
           << strDefault.c_str()            << ";"
           << std::endl;
    }

    static void printEnum(
        const RawMessage::KeyValueMap & map,
        std::ostream & os,
        int indent = 0
    ) {
        for (int i = 0; i < indent; ++i) os << '\t';
        os << "enum " << RawMessage::At(map,1)->asString().c_str() << " {" << std::endl;

        // values
        const RawMessage::VariantPtr vaItem = RawMessage::At(map,2);
        if (!vaItem->isRepeated()) {
            for (int i = 0; i <= indent; ++i) os << '\t';
            const RawMessage::KeyValueMap vit = vaItem->asMap();
            os << RawMessage::At(vit,1)->asString().c_str() << " = "
               << RawMessage::At(vit,2)->asInt()            << ";"
               << std::endl;
        } else {
            const RawMessage::KeyValueMap & v = vaItem->asMap();
            for (RawMessage::KeyValueMap::const_iterator it = v.begin(); it != v.end(); ++it) {
                const RawMessage::KeyValueMap & vit = it->second->asMap();
                for (int i = 0; i <= indent; ++i) os << '\t';
                os << RawMessage::At(vit,1)->asString().c_str() << " = "
                   << RawMessage::At(vit,2)->asInt()            << ";"
                   << std::endl;

            }
        }
        for (int i = 0; i < indent; ++i) os << '\t';
        os << '}' << std::endl;
    }

    static void printMessage(
        const RawMessage::VariantPtr & var,
        std::ostream & os,
        int indent = 0
    ) {
        assert(var->isMap());
        const RawMessage::KeyValueMap & map = var->asMap();

        for (int i = 0; i < indent; ++i) os << '\t';
        os << "message " << RawMessage::At(map,1)->asString().c_str() << " {" << std::endl;

        // enums
        if (var->hasField(4)) {
            const RawMessage::VariantPtr vaItem = RawMessage::At(map,4);
            if (vaItem->isMap()) {
                printEnum(vaItem->asMap(), os, indent + 1);
            } else {
                const RawMessage::KeyValueMap & m = vaItem->asMap();
                for (RawMessage::KeyValueMap::const_iterator it = m.begin(); it != m.end(); ++it) {
                    printEnum(it->second->asMap(), os, indent + 1);
                }
            }
        }

        // sub messages
        if (var->hasField(3)) {
            const RawMessage::VariantPtr vaItem = RawMessage::At(map,3);
            if (vaItem->isMap()) {
                printMessage(vaItem, os, indent + 1);
            } else {
                const RawMessage::KeyValueMap & m = vaItem->asMap();
                for (RawMessage::KeyValueMap::const_iterator it = m.begin(); it != m.end(); ++it) {
                    printMessage(it->second, os, indent + 1);
                }
            }
        }

        // items of current message
        if (var->hasField(2)) {
            const RawMessage::VariantPtr vaItem = RawMessage::At(map,2);
            if (vaItem->isMap()) {
                printField(vaItem->asMap(), os, indent + 1);
            } else {
                const RawMessage::KeyValueMap & m = vaItem->asMap();
                for (RawMessage::KeyValueMap::const_iterator it = m.begin(); it != m.end(); ++it) {
                    printField(it->second->asMap(), os, indent + 1);
                }
            }
        }

        for (int i = 0; i < indent; ++i) os << '\t';
        os << '}' << std::endl;
    }

    static bool isSerializedMessages(const RawMessage & msg) {
        RawMessage::KeyValueMap & items = msg.rootItem()->asMap();
        bool f = items.count(1) &&
                 items.count(2) &&
                 items.count(4);
        if (f) {
            f = items[1]->isString() &&
                items[2]->isString() &&
                (items[4]->isMap() || items[4]->isRepeated());
        }
        return f;
    }

public:
    static unsigned grab(
        const unsigned char * ptr,
        const unsigned char * ept
    ) {
        RawMessage msg;
        const unsigned char * ptrBegin = ptr;
        const unsigned char * ptrEnd   = ept;
        unsigned count = 0;
        while (ptr < ept) {
            ptr = findSerializedPB(ptr, ept);
            if (!ptr) break;
#if DEBUG
            std::cerr << "offs 0x" << std::hex << (std::ptrdiff_t) (ptr - 0)
                      <<     " 0x" << std::hex << (ptr - ptrBegin)
                      <<     " 0x" << std::hex << (ept - ptrBegin)
                      << std::endl;
#endif
            if (msg.parse(ptr, ept) && isSerializedMessages(msg)) {
#if DEBUG
                msg.print(std::cerr);
#endif
                std::string filename(msg.items()[1]->asString());
#if WIN32
                std::replace(filename.begin(), filename.end(), '/', '\\');
#endif
                std::ofstream file(filename.c_str(), std::ios::binary);
                if (!file.is_open()) {
                    std::cout << " [-] " << filename.c_str()
                              << " ERROR: can't create file path!"
                              << std::endl;
                } else {
                    printMessagesFromSerialized(msg, file);
                    std::cout << " [+] " << filename.c_str() << std::endl;
                    count += 1;
                }
            }
            ptr = ept + 1;
            ept = ptrEnd;
        }
        return count;
    }

    static void printMessagesFromSerialized(const RawMessage & msg, std::ostream & os, bool force = false) {
        if (!force && !isSerializedMessages(msg)) {
            return;
        }
        // package name
        if (msg.rootItem()->asMap().count(2)) {
            std::string packageName(msg.rootItem()->asMap()[2]->asString());
            os << "package " << packageName.c_str() << ";" << std::endl;
        }
        // imports
        if (msg.rootItem()->asMap().count(3)) {
            const RawMessage::VariantPtr vaItem = msg.rootItem()->asMap()[3];
            if (!vaItem->isRepeated()) {
                os << "import \"" << vaItem->asString().c_str() << "\";\n";
            } else {
                const RawMessage::KeyValueMap & m = vaItem->asMap();
                for (RawMessage::KeyValueMap::const_iterator it = m.begin(); it != m.end(); ++it) {
                    os << "import \"" << it->second->asString().c_str() << "\";\n";
                }
            }
        }
        // enums
        if (msg.rootItem()->asMap().count(5)) {
            const RawMessage::VariantPtr vaItem = msg.rootItem()->asMap()[5];
            if (vaItem->isMap()) {
                printEnum(vaItem->asMap(), os, 0);
            } else {
                const RawMessage::KeyValueMap & m = vaItem->asMap();
                for (RawMessage::KeyValueMap::const_iterator it = m.begin(); it != m.end(); ++it) {
                    printEnum(it->second->asMap(), os, 0);
                }
            }
        }
        // messages
        if (msg.rootItem()->asMap().count(4)) {
            const RawMessage::VariantPtr vaItem = msg.rootItem()->asMap()[4];
            if (vaItem->isMap()) {
                printMessage(vaItem, os);
            } else {
                const RawMessage::KeyValueMap & m = vaItem->asMap();
                for (RawMessage::KeyValueMap::const_iterator it = m.begin(); it != m.end(); ++it) {
                    printMessage(it->second, os);
                }
            }
        }
    }

    static const unsigned char * findSerializedPB(
        const unsigned char * p,
        const unsigned char *& e
    ) {
        const unsigned char * b, *start = p, *endPtr;
        for (;;) {
            // 0a:VARINT:STRING
            // find first field
            for (; p < e && *p != 0x0a; ++p);
            if (p >= e) break;

            endPtr = p+1;
            bool isValid = false;
            for (int tr = 0; tr < 10; ++tr, ++endPtr) {
                // find next '\0' after protobuf message
                for (; endPtr < e-1 && *endPtr; ++endPtr);
                if (endPtr >= e-1) break;

                // filename field
#if DEBUG
                //std::cerr << "(1) " << std::hex << (p - start) << std::endl;
#endif
                int64_t v = 0;
                b = RawMessage::readVarint(p+1, endPtr, v);
                if (b >= endPtr || b+v >= endPtr) {
#if DEBUG
                    //std::cerr << "(1) NOPE " << std::hex << (e - b) << " " << (b - start) << std::endl;
#endif
                    continue;
                }
                if (v <= 0 || b[v] != 0x12 || !RawMessage::itsAsciiString(b, b+v)) {
#if DEBUG
                    //std::cerr << "(1) +NOPE" << std::endl;
#endif
                    break;
                }
                // 12:VARINT:STRING
                // namespace field
#if DEBUG
                std::cerr << "(2) " << std::hex << (p - start) << std::endl;
#endif
                b = RawMessage::readVarint(b+v+1, endPtr, v);
                if (b >= endPtr || b+v >= endPtr) {
#if DEBUG
                    std::cerr << "(2) NOPE" << std::endl;
#endif
                    continue;
                }
                if (v <= 0 || !RawMessage::itsAsciiString(b, b+v)) {
#if DEBUG
                    std::cerr << "(2) +NOPE" << std::endl;
#endif
                    break;
                }

                if (RawMessage::isValidMessage(p, endPtr)) {
                    isValid = true;
                    break;
                }
            }
            if (!isValid) {
                ++p;
                continue;
            }
            // found
#if DEBUG
            std::cerr << "FOUND" << std::endl;
#endif
            e = endPtr;
            return p;
        }
        return NULL;
    }
};

// /////////////////////////////////////////////////////////////////// //

class Schema {
public:
    static void print(const RawMessage & message, std::ostream & os) {
        std::vector< RawMessage::VariantPtr > messages;
        std::map< std::string, unsigned > lookupContext;
        fillSchemasInternal(message.rootItem(), messages, lookupContext);
        os << "package ProtodecMessages;\n";
        for (unsigned i = 0; i < messages.size(); ++i) {
            os << "\nmessage MSG" << (i+1) << " {\n";
            os << messages[i]->asString();
            os << "}\n";
        }
    }

private:
    static void fillSchemasInternal(
        const RawMessage::VariantPtr & message,
        std::vector< RawMessage::VariantPtr > & messages,
        std::map< std::string, unsigned > & lookupContext
    ) {
        std::stringstream ss;
        const RawMessage::KeyValueMap & map = message->asMap();
        for (RawMessage::KeyValueMap::const_iterator it = map.begin(); it != map.end(); ++it) {
            ss << "\t";
            const RawMessage::VariantPtr & var = it->second;
            if (!var->isRepeated()) {
                if (var->isMap()) {
                    assert(!var->asMap().empty());
                    fillSchemasInternal(var, messages, lookupContext);
                }
                ss << "required " << var->dataType();
            } else {
                assert(!var->asMap().empty());
                const RawMessage::VariantPtr & subVar = (var->asMap().begin())->second;
                if (subVar->isMap() || subVar->isRepeated()) {
                    fillSchemasInternal(subVar, messages, lookupContext);
                }
                ss << "repeated " << subVar->dataType();
            }
            ss << " fld" << (it->first) << " = " << (it->first) << ";\n";
        }
        // store key and current message in lookupContext for next matches
        message->asString() = ss.str();
        if (lookupContext.count(message->asString()) != 0) {
            message->setGlobalId(lookupContext[message->asString()]);
        } else {
            messages.push_back(message);
            message->setGlobalId(messages.size());
            lookupContext[message->asString()] = messages.size();
        }
    }
}; // Schema

// /////////////////////////////////////////////////////////////////// //

inline std::ostream& operator<<(std::ostream & os, const RawMessage::Variant & var) {
    if (var.isInt()) {
        os /*<< "int64:"*/ << var.asInt();
    } else if (var.isFloat()) {
        os /*<< "float:"*/ << var.asFloat();
    } else if (var.isDouble()) {
        os /*<< "double:"*/ << var.asDouble();
    } else if (var.isString()) {
        os << '"';
        const std::string & str = var.asString();
        for (unsigned i = 0; i < str.length(); ++i) {
            unsigned char ch = (unsigned char) str[i];
            if (isascii(ch) && ch != 5 && ch != 00) {
                os << str[i];
            } else {
                os << '\\';
                if (ch < 100) os << '0';
                if (ch <  99) os << '0';
                os << (int)ch;
            }
        }
        os << '"';
    } else {
        assert(!"This shouldn't happen.");
    }
    return os;
}
