#include "utl/text/MakeString.h"

#include <cstring>

#include "os/debug/Debug.h"
#include "os/memory/MemMgr.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/text/HmxSnprintf.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// The calling thread's MakeString buffers. In this build the thread data
// lives in the engine's shared per-thread block (created at 0x259570 and
// found through the descriptor returned by 0x258B80); this object sits at
// offset 0x3430, recorded in 0x19B0398 by the static initializer at
// 0x248520. The map's build keeps it in a TLSValue<TlsMkStringData>.
struct TlsMkStringData {
    // Reconstructed from eboot.elf at 0x2596C0. The map places it in
    // MakeString.o.
    TlsMkStringData() {
        mNextBuf = 0;
        mBufs.resize(kNumBufs);
        for (unsigned long i = 0; i < kNumBufs; ++i) {
            mBufs[i].resize(kBufSize);
        }
    }

    static constexpr unsigned long kNumBufs = 16;    // Name not in the reference map.
    static constexpr unsigned long kBufSize = 4096;  // Name not in the reference map.

    // Field names are not in the reference map.
    unsigned long mNextBuf;  // The ring index of the next buffer.
    eastl::vector<eastl::vector<char>> mBufs;
};

static_assert(sizeof(TlsMkStringData) == 40);

namespace {

// Name not in the reference map.
thread_local TlsMkStringData tMkStringData;

}  // namespace

// The calling thread's format slots, one for each FormatString alive on it.
// In this build it follows TlsMkStringData in the shared per-thread block
// (offset 0x3458) and the creator zeroes the depth.
struct FormatBuf {
    // Field names are not in the reference map.
    char mFmt[2][4096];
    int mDepth;  // The number of FormatStrings using a slot.
};

static_assert(sizeof(FormatBuf) == 8196);

// The map's build keeps it in a TLSValue<FormatBuf> of this name.
thread_local FormatBuf tFormatBuf;

namespace {

// Set the first time a format overflows its buffer, at 0x19E7CB4: the
// remnant of a notify-once. Name not in the reference map.
bool gFormatOverflowed;

}  // namespace

// Reconstructed from eboot.elf at 0x2471A0.
char* MakeStringBuf() {
    // The first access creates the thread's data, which must not come from
    // the temporary heap.
    unsigned int saved;
    MemPushTemp(saved, false, true);
    TlsMkStringData& data = tMkStringData;
    MemPopTemp(saved);

    const unsigned long index = data.mNextBuf;
    char* buf = data.mBufs[index].data();
    data.mNextBuf = index + 1 != TlsMkStringData::kNumBufs ? index + 1 : 0;
    buf[0] = '\0';
    return buf;
}

// Reconstructed from eboot.elf at 0x247290.
FormatString::FormatString()
    : mType(kFormatNone),
      mBuf(MakeStringBuf()),
      mBufSize(4096),
      mNextFmt(nullptr),
      mFmtBuf(nullptr) {}

// Reconstructed from eboot.elf at 0x2472C0.
FormatString::FormatString(const char* fmt)
    : mType(kFormatNone),
      mBuf(MakeStringBuf()),
      mBufSize(4096),
      mNextFmt(nullptr),
      mFmtBuf(nullptr) {
    InitializeWithFmt(fmt, nullptr, true);
}

// Reconstructed from eboot.elf at 0x247310.
void FormatString::InitializeWithFmt(const char* fmt, char* buf, bool updateType) {
    static_cast<void>(buf);

    unsigned int saved;
    MemPushTemp(saved, false, true);
    FormatBuf& formatBuf = tFormatBuf;
    MemPopTemp(saved);

    const int depth = formatBuf.mDepth;
    mFmtBuf = formatBuf.mFmt[depth];
    formatBuf.mDepth = depth + 1;
    if (depth >= 2) {
        HmxFail("double failure");
    }
    std::memcpy(mFmtBuf, fmt, std::strlen(fmt) + 1);
    mFmt = mFmtBuf;
    if (updateType) {
        _UpdateType();
    }
}

// Reconstructed from eboot.elf at 0x2474F0.
FormatString::FormatString(const FormatString& other) {
    static_cast<void>(other);
}

// Reconstructed from eboot.elf at 0x247500.
FormatString& FormatString::operator=(const FormatString& other) {
    static_cast<void>(other);
    return *this;
}

// Reconstructed from eboot.elf at 0x247510.
FormatString::~FormatString() {
    const int depth = tFormatBuf.mDepth;
    tFormatBuf.mDepth = depth - 1;
    if (depth <= 0) {
        HmxFail("double failure");
    }
}

// Reconstructed from eboot.elf at 0x2475C0.
void FormatString::_UpdateType() {
    char* fmt = mFmt;
    if (mNextFmt != nullptr) {
        mFmt = fmt = mNextFmt;
        mNextFmt = nullptr;
    }

    // 0: no specifier yet; 1: inside a specifier; 2: its type is known.
    int state = 0;
    for (;;) {
        const char c = *fmt;
        if (c == '%') {
            if (fmt[1] == '%') {
                fmt += 2;
                continue;
            }
            if (state != 0) {
                mNextFmt = fmt;
                return;
            }
            state = 1;
            ++fmt;
            continue;
        }
        if (c == '\0') {
            break;
        }
        if (state == 1 && static_cast<unsigned char>(c - 'A') <= 'z' - 'A') {
            if (c == 'a' || c == 'f' || c == 'g') {
                mType = kFormatFloat;
            } else {
                mType = c == 's' ? kFormatStr : kFormatInt;
            }
            state = 2;
        }
        ++fmt;
    }
    if (state == 0) {
        mType = kFormatNone;
    }
    mNextFmt = fmt;
}

template <typename T>
FormatString& FormatString::Format(T value) {
    // Ends the format at the next specifier while formatting.
    const char next = *mNextFmt;
    *mNextFmt = '\0';
    const int length = HmxSnprintf(mBuf + 4096 - mBufSize, mBufSize, mFmt, value);
    *mNextFmt = next;
    if (length < 0 && !gFormatOverflowed) {
        gFormatOverflowed = true;
    }
    mBufSize -= length;
    _UpdateType();
    return *this;
}

// Reconstructed from eboot.elf at 0x247670.
FormatString& FormatString::operator<<(void* value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x247780.
FormatString& FormatString::operator<<(unsigned int value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x247890.
FormatString& FormatString::operator<<(unsigned long value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x2479A0.
FormatString& FormatString::operator<<(long value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x247AB0.
FormatString& FormatString::operator<<(unsigned long long value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x247BC0.
FormatString& FormatString::operator<<(long long value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x247CD0.
FormatString& FormatString::operator<<(int value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x247DE0.
FormatString& FormatString::operator<<(const DataNode& value) {
    const char next = *mNextFmt;
    *mNextFmt = '\0';
    char* const out = mBuf + 4096 - mBufSize;
    int length;
    switch (mType) {
    case kFormatInt:
        length = HmxSnprintf(
            out, mBufSize, mFmt,
            value.mType == kDataFloat ? static_cast<int>(value.mValue.real)
                                      : value.mValue.integer);
        break;
    case kFormatFloat:
        length = HmxSnprintf(
            out, mBufSize, mFmt,
            static_cast<double>(value.mType != kDataInt
                                    ? value.mValue.real
                                    : static_cast<float>(value.mValue.integer)));
        break;
    case kFormatStr:
        // Symbols and resource paths hold their text; strings keep it in
        // their array's node pointer.
        length = HmxSnprintf(
            out, mBufSize, mFmt,
            value.mType == kDataResourcePath || value.mType == kDataSymbol
                ? value.mValue.symbol
                : reinterpret_cast<const char*>(value.mValue.array->mNodes));
        break;
    default:
        length = 0;
        break;
    }
    *mNextFmt = next;
    mBufSize -= length;
    _UpdateType();
    return *this;
}

// Reconstructed from eboot.elf at 0x247F90.
FormatString& FormatString::operator<<(const char* value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x2480A0.
FormatString& FormatString::operator<<(float value) {
    return Format(static_cast<double>(value));
}

// Reconstructed from eboot.elf at 0x2481B0.
FormatString& FormatString::operator<<(double value) {
    return Format(value);
}

// Reconstructed from eboot.elf at 0x2482C0.
FormatString& FormatString::operator<<(const FixedString& value) {
    return Format(value.c_str());
}

// Reconstructed from eboot.elf at 0x2483D0.
FormatString& FormatString::operator<<(const Symbol& value) {
    return Format(value.Str());
}

// Reconstructed from eboot.elf at 0x2484E0.
const char* FormatString::Str() {
    if (*mFmt != '\0') {
        std::strcpy(mBuf + 4096 - mBufSize, mFmt);
    }
    return mBuf;
}
