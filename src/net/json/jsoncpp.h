#pragma once

#include <cstddef>

// The JSON library the server jobs read and write (net/jsoncpp.o, the
// open-source JsonCpp). Only the layout and the members the reconstructed
// code calls are declared; the object is not reconstructed.
namespace Json {

// The kinds of value.
enum ValueType {
    nullValue = 0,
    intValue,
    uintValue,
    realValue,
    stringValue,
    booleanValue,
    arrayValue,
    objectValue,
};

// A JSON value (40 bytes): the payload, its kind, its comments and its place
// in the parsed text.
class Value {
public:
    ~Value();  // 0x346DE0

    // The member of an object; the null value when it is missing.
    const Value& operator[](const char* key) const;  // 0x3494E0
    // The element of an array.
    const Value& operator[](unsigned int index) const;  // 0x348D80
    // A copy of the member, or of `defaultValue` when it is missing.
    Value get(const char* key, const Value& defaultValue) const;  // 0x349770

    const char* asCString() const;  // 0x347280
    int asInt() const;  // 0x3476C0
    long long asInt64() const;  // 0x347C70
    unsigned long long asUInt64() const;  // 0x347B00

    bool isNull() const;  // 0x3485D0
    bool isArray() const;  // 0x3411C0
    // The elements of an array or the members of an object.
    unsigned int size() const;  // 0x3484B0

    // The shared null value (the reference at 0x18FA820 to 0x1276DE0).
    static const Value& null;

private:
    // Field names follow JsonCpp's.
    union ValueHolder {
        long long int_;
        unsigned long long uint_;
        double real_;
        bool bool_;
        char* string_;
        void* map_;
    } value_;
    unsigned char type_;
    unsigned char allocated_;
    void* comments_;
    unsigned long start_;
    unsigned long limit_;
};

static_assert(sizeof(Value) == 40);

}  // namespace Json
