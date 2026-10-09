#pragma once

#include "utl/data/DataArray.h"

// A script message whose array is (target type args...). The map's build
// instantiates it for up to five arguments (Message<0> to Message<5>); all
// its members are inline. A message built from its arguments keeps the
// array in its own storage: mStorage over the N + 2 nodes that follow it.
template <int N>
class Message {
public:
    // Wraps an existing message array, holding a reference.
    explicit Message(DataArray* data) : mData(data), mStorage(0) {
        mData->AddRef();
    }
    // The binary keeps Message<5>'s at 0x392150 (complete destructor thunk
    // 0x392120, deleting destructor 0x392130).
    virtual ~Message() {
        if (mData == &mStorage) {
            // The nodes are the message's own.
            mStorage.mNodes = nullptr;
            mStorage.mSize = 0;
        } else {
            mData->Release();
        }
    }

    // Field names are not in the reference map.
    DataArray* mData;
    DataArray mStorage;
    DataNode mNodes[N + 2];
};
