#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

class DataArray;

// Type tag of a DataNode. The name is the map's; the enumerators follow the
// Milo engine and are not in the reference map. Types with kDataArray set
// hold a reference-counted DataArray; the two waveform types hold a
// reference-counted waveform.
enum DataType : int {
    kDataInt = 0,
    kDataFloat = 1,
    kDataVar = 2,
    kDataFunc = 3,
    kDataObject = 4,
    kDataSymbol = 5,
    kDataUnhandled = 6,
    kDataArray = 0x10,
    kDataCommand = 0x11,
    kDataString = 0x12,  // A DataArray whose nodes are the characters.
    kDataProperty = 0x13,
    kDataWaveformFloat = 0x26,
    kDataWaveformColor = 0x27,
};

// Value of a DataNode. Name not in the reference map.
union DataNodeValue {
    int integer;
    float real;
    const char* symbol;
    DataArray* array;
    void* object;
};

// A typed script value. Copies share arrays and waveforms by reference.
class DataNode {
public:
    explicit DataNode(int value) : mType(kDataInt) {
        mValue.object = nullptr;
        mValue.integer = value;
    }
    DataNode(const DataNode& other);
    ~DataNode();
    DataNode& operator=(const DataNode& other);

    DataType Type() const {
        return mType;
    }

    // The value as an integer, float, symbol or string, evaluating
    // variables, commands and properties. The source array names the file
    // and line in errors.
    int Int(const DataArray* source) const;
    float Float(const DataArray* source) const;
    Symbol Sym(const DataArray* source) const;
    const char* Str(const DataArray* source) const;

    void AddRefWaveform() const;   // 0x239290
    void ReleaseWaveform() const;  // 0x2392A0

    DataNodeValue mValue;
    DataType mType;
};

static_assert(sizeof(DataNode) == 16);

#include "utl/data/DataArray.h"
