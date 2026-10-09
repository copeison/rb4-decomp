#include "utl/text/StringTable.h"

#include <cstring>

#include "os/memory/MemMgr.h"

// Reconstructed from eboot.elf at 0x25F860.
StringTable::StringTable(const char* name, unsigned long size, unsigned long align)
    : mAlign(align), mCurBuf(-1), mName(name) {
    if (size != 0) {
        _AddBuf(size);
    }
}

// Reconstructed from eboot.elf at 0x25F8D0.
void StringTable::_AddBuf(unsigned long size) {
    auto* start = static_cast<char*>(MemAlloc(size, "StringTable", 0));
    mCurBuf = mBuffers.size();
    mBuffers.push_back(Buf{size, start, start});
}

// Reconstructed from eboot.elf at 0x25F9F0.
StringTable::~StringTable() {
    for (unsigned long i = 0; i < mBuffers.size(); ++i) {
        MemFree(mBuffers[i].mStart);
    }
}

// Reconstructed from eboot.elf at 0x25FC50.
const char* StringTable::Add(const char* str) {
    const unsigned long length = strlen(str) + 1;
    if (mCurBuf == -1) {
        _AddBuf(length > 0x100 ? length : 0x100);
    } else {
        Buf& current = mBuffers[mCurBuf];
        if (current.mFree + length - current.mStart > static_cast<long>(current.mSize)) {
            const long next = mCurBuf + 1;
            if (next >= static_cast<long>(mBuffers.size())) {
                // Grow by as much as the table holds.
                unsigned long total = 0;
                for (unsigned long i = 0; i < mBuffers.size(); ++i) {
                    total += mBuffers[i].mSize;
                }
                _AddBuf(total < length ? length : total);
            } else {
                mCurBuf = next;
                Buf& buffer = mBuffers[next];
                if (buffer.mSize < length) {
                    MemFree(buffer.mStart);
                    buffer.mSize = length;
                    buffer.mStart = static_cast<char*>(MemAlloc(length, "StringTable", 0));
                    buffer.mFree = buffer.mStart;
                }
            }
        }
    }
    Buf& buffer = mBuffers[mCurBuf];
    memcpy(buffer.mFree, str, length);
    char* copy = buffer.mFree;
    buffer.mFree += length;
    const unsigned long misalignment = reinterpret_cast<unsigned long>(buffer.mFree) % mAlign;
    if (misalignment != 0) {
        const unsigned long padding = mAlign - misalignment;
        memset(buffer.mFree, 0, padding);
        buffer.mFree += padding;
    }
    return copy;
}

// Reconstructed from eboot.elf at 0x25FDC0.
unsigned long StringTable::UsedSize() const {
    unsigned long used = 0;
    for (unsigned long i = 0; i < mBuffers.size(); ++i) {
        used += mBuffers[i].mFree - mBuffers[i].mStart;
        if (static_cast<long>(i) == mCurBuf) {
            break;
        }
    }
    return used;
}

// Reconstructed from eboot.elf at 0x25FE10.
bool StringTable::Contains(const char* str) const {
    const unsigned long length = strlen(str);
    for (unsigned long i = 0; i < mBuffers.size(); ++i) {
        const char* end = mBuffers[i].mFree;
        for (const char* entry = mBuffers[i].mStart; entry < end;) {
            const unsigned long entryLength = strlen(entry);
            if (entryLength == length && strcmp(str, entry) == 0) {
                return true;
            }
            entry += entryLength + 1;
        }
        if (static_cast<long>(i) == mCurBuf) {
            break;
        }
    }
    return false;
}
