#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

// A reference-counted array of DataNodes, parsed from a script file. A
// string node keeps its characters in an array with a negative size.
class DataArray {
public:
    POOL_OVERLOAD(DataArray)

    ~DataArray();  // 0x21E500

    int Size() const {
        return mSize;
    }
    DataNode& Node(unsigned long index) {
        return mNodes[index];
    }
    const DataNode& Node(unsigned long index) const {
        return mNodes[index];
    }
    // The node at the index, with variables, commands and properties
    // evaluated.
    DataNode Evaluate(unsigned long index) const;  // 0xC7D30

    void AddRef() {
        __atomic_add_fetch(&mRefs, 1, __ATOMIC_SEQ_CST);
    }
    // The binary also has an out-of-line copy at 0x21FF70.
    void Release() {
        if (__atomic_fetch_sub(&mRefs, 1, __ATOMIC_SEQ_CST) == 1) {
            delete this;
        }
    }

    // Field names follow the Milo engine.
    DataNode* mNodes;
    Symbol mFile;
    int mRefs;
    short mSize;
    short mLine;
};

static_assert(sizeof(DataArray) == 24);

inline DataNode::DataNode(const DataNode& other) : mValue(other.mValue), mType(other.mType) {
    if ((mType & kDataArray) != 0) {
        mValue.array->AddRef();
    } else if ((mType & ~1) == kDataWaveformFloat) {
        AddRefWaveform();
    }
}

inline DataNode::~DataNode() {
    if ((mType & kDataArray) != 0) {
        mValue.array->Release();
    }
    if ((mType & ~1) == kDataWaveformFloat) {
        ReleaseWaveform();
    }
}

inline DataNode& DataNode::operator=(const DataNode& other) {
    if ((other.mType & kDataArray) != 0) {
        other.mValue.array->AddRef();
    } else if ((other.mType & ~1) == kDataWaveformFloat) {
        other.AddRefWaveform();
    }
    if ((mType & kDataArray) != 0) {
        mValue.array->Release();
    }
    if ((mType & ~1) == kDataWaveformFloat) {
        ReleaseWaveform();
    }
    mValue = other.mValue;
    mType = other.mType;
    return *this;
}
