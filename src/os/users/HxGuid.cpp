#include "os/users/HxGuid.h"

#include <string.h>

#include "math/hash/SHA1.h"
#include "math/random/Rand.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/text/TextStream.h"
#include "utl/time/DateTime.h"

namespace {

// The value of a hexadecimal digit. Inlined into operator=. Name not in the
// reference map.
int HexDigitValue(char c) {
    int value = c - '0';
    if (c > 'A') {
        value = c - 'A' + 10;
    }
    if (c > '`') {
        value = c - 'a' + 10;
    }
    return value;
}

}  // namespace

// Reconstructed from eboot.elf at 0x246960.
HxGuid::HxGuid() {
    Clear();
}

// Reconstructed from eboot.elf at 0x246970.
void HxGuid::Clear() {
    mData[0] = 0;
    mData[1] = 0;
    mData[2] = 0;
    mData[3] = 0;
}

// Reconstructed from eboot.elf at 0x246980.
void HxGuid::Generate() {
    CSHA1 sha;
    DateTime now = GetDateAndTime();
    sha.Update(&now, sizeof(now));
    unsigned long random = gRand.Int();
    sha.Update(&random, sizeof(random));
    unsigned long cycles = __builtin_ia32_rdtsc();
    sha.Update(&cycles, sizeof(cycles));
    memcpy(mData, sha.Final().mData, sizeof(mData));
}

// Reconstructed from eboot.elf at 0x246A80.
bool HxGuid::IsNull() const {
    return mData[0] == 0 && mData[1] == 0 && mData[2] == 0 && mData[3] == 0;
}

// Reconstructed from eboot.elf at 0x246AB0.
int HxGuid::Chunk32(int index) const {
    return mData[index];
}

// Reconstructed from eboot.elf at 0x246AC0.
void HxGuid::SetChunk32(int index, int value) {
    mData[index] = value;
}

// Reconstructed from eboot.elf at 0x246AD0.
const char* HxGuid::ToString() const {
    FormatString text("%08x%08x%08x%08x");
    return (text << mData[0] << mData[1] << mData[2] << mData[3]).Str();
}

// Reconstructed from eboot.elf at 0x246B60.
const char* HxGuid::ToShortString() const {
    FormatString text("%04x");
    text << static_cast<unsigned short>(mData[3]);
    return text.Str();
}

// Reconstructed from eboot.elf at 0x246BD0.
HxGuid& HxGuid::operator=(String str) {
    for (int chunk = 0; chunk < 4; ++chunk) {
        String digits = str.substr(chunk * 8, 8);
        int value = 0;
        for (int digit = 7; digit >= 0; --digit) {
            value += HexDigitValue(digits[digit]) << ((7 - digit) * 4);
        }
        mData[chunk] = value;
    }
    return *this;
}

// Reconstructed from eboot.elf at 0x246E00.
bool HxGuid::operator==(const HxGuid& other) const {
    return mData[0] == other.mData[0] && mData[1] == other.mData[1] && mData[2] == other.mData[2]
        && mData[3] == other.mData[3];
}

// Reconstructed from eboot.elf at 0x246E30.
bool HxGuid::operator!=(const HxGuid& other) const {
    return mData[0] != other.mData[0] || mData[1] != other.mData[1] || mData[2] != other.mData[2]
        || mData[3] != other.mData[3];
}

// Reconstructed from eboot.elf at 0x246E60.
bool HxGuid::operator<(const HxGuid& other) const {
    if (mData[0] != other.mData[0]) {
        return mData[0] < other.mData[0];
    }
    if (mData[1] != other.mData[1]) {
        return mData[1] < other.mData[1];
    }
    if (mData[2] != other.mData[2]) {
        return mData[2] < other.mData[2];
    }
    return mData[3] < other.mData[3];
}

// Reconstructed from eboot.elf at 0x246EA0.
const int* HxGuid::Data() const {
    return mData;
}

// Reconstructed from eboot.elf at 0x246EB0.
void HxGuid::SetData(int chunk0, int chunk1, int chunk2, int chunk3) {
    mData[0] = chunk0;
    mData[1] = chunk1;
    mData[2] = chunk2;
    mData[3] = chunk3;
}

// Reconstructed from eboot.elf at 0x246EC0.
int HxGuid::SaveSize() {
    return 20;
}

// Reconstructed from eboot.elf at 0x246ED0.
const HxGuid& HxGuid::NULL_GUID() {
    static HxGuid sNullGuid;
    return sNullGuid;
}

// Reconstructed from eboot.elf at 0x246F10. The stream's int writes are
// inline.
BinStream& operator<<(BinStream& stream, const HxGuid& guid) {
    int value = 1;
    stream.WriteEndian(&value, sizeof(value));
    for (int chunk = 0; chunk < 4; ++chunk) {
        value = guid.mData[chunk];
        stream.WriteEndian(&value, sizeof(value));
    }
    return stream;
}

// Reconstructed from eboot.elf at 0x246FD0.
BinStream& operator>>(BinStream& stream, HxGuid& guid) {
    int revision;
    stream >> revision;
    stream >> guid.mData[0] >> guid.mData[1] >> guid.mData[2] >> guid.mData[3];
    return stream;
}

// Reconstructed from eboot.elf at 0x247070.
TextStream& operator<<(TextStream& stream, const HxGuid& guid) {
    stream << guid.ToString();
    return stream;
}
