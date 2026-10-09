#pragma once

#include <cstddef>

#include "math/hash/SHA1.h"
#include "os/memory/MemMgr.h"
#include "utl/text/Str.h"

// SHA-1 checksum of the bytes read through a stream. Only the members reached
// by recovered code are modeled.
class StreamChecksum {
public:
    ~StreamChecksum() = default;

    // SHA-1 update at 0x367C50.
    void Update(const unsigned char* data, unsigned long size);

    DELETE_OVERLOAD

    // 0 before the first update, 1 while hashing (Update at 0x367C50
    // resets the hash when it is 0 or 2) and 2 once the hash is final
    // (0x367CB0). Name not in the reference map.
    int mState;
    unsigned char mPadding[4];  // Never read or written.
    CSHA1 mSha1;      // Name not in the reference map.
    String mFile;     // Name not in the reference map.
};

static_assert(offsetof(StreamChecksum, mSha1) == 0x08);
static_assert(offsetof(StreamChecksum, mFile) == 0xD8);
