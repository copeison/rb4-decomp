#include "utl/streams/BinStream.h"

#include <cstring>

#include "math/random/Rand2.h"
#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"

namespace {

constexpr unsigned long kCipherChunkSize = 512;

template <typename Value>
void SwapInPlace(void* data) {
    Value value;
    std::memcpy(&value, data, sizeof(value));
    Value swapped = 0;
    for (std::size_t index = 0; index < sizeof(Value); ++index) {
        swapped = static_cast<Value>(
            (swapped << 8) | ((value >> (index * 8)) & 0xFF));
    }
    std::memcpy(data, &swapped, sizeof(swapped));
}

void SwapValue(void* data, int size) {
    switch (size) {
    case 8:
        SwapInPlace<std::uint64_t>(data);
        break;
    case 4:
        SwapInPlace<std::uint32_t>(data);
        break;
    case 2:
        SwapInPlace<std::uint16_t>(data);
        break;
    default:
        break;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x21A680.
BinStream::BinStream(bool littleEndian, int platform)
    : mSkipStart(-1),
      mSkipSize(-1),
      mStreamFailed(false),
      mLittleEndian(littleEndian),
      mCrypto(nullptr),
      mPlatform(platform) {}

// Reconstructed from eboot.elf at 0x21A6B0. The stream owns its cipher. The
// deleting destructor at 0x21A6D0 releases the stream through MemFree.
BinStream::~BinStream() {
    if (mCrypto != nullptr) {
        MemFree(mCrypto);
    }
}

void BinStream::Decrypt(void* data, unsigned long size) {
    if (mCrypto == nullptr || size == 0) {
        return;
    }
    auto* bytes = static_cast<std::uint8_t*>(data);
    for (unsigned long index = 0; index < size; ++index) {
        bytes[index] ^= static_cast<std::uint8_t>(mCrypto->Int());
    }
}

// Encrypted writes pass through a 512-byte stack buffer in chunks.
void BinStream::WriteEncrypted(const void* data, unsigned long size) {
    std::uint8_t chunk[kCipherChunkSize];
    const auto* source = static_cast<const std::uint8_t*>(data);
    while (size != 0) {
        const auto count = size < kCipherChunkSize ? size : kCipherChunkSize;
        for (unsigned long index = 0; index < count; ++index) {
            chunk[index] =
                static_cast<std::uint8_t>(mCrypto->Int() ^ source[index]);
        }
        source += count;
        WriteImpl(chunk, count);
        size -= count;
    }
}

// Reconstructed from eboot.elf at 0x21A280. The fail query is issued before
// every read and its result is ignored, matching the original.
void BinStream::Read(void* data, unsigned long size) {
    Fail();
    ReadImpl(data, size);
    Decrypt(data, size);
}

// Reconstructed from eboot.elf at 0x21ABF0. Byte-swapped streams reverse 2-,
// 4-, and 8-byte values after decryption.
void BinStream::ReadEndian(void* data, int size) {
    Read(data, static_cast<unsigned long>(size));
    if (mLittleEndian != 0) {
        SwapValue(data, size);
    }
}

// Reconstructed from eboot.elf at 0x21A300. Symbols are stored as a 32-bit
// length followed by unterminated text. Text of up to 1023 bytes is staged on
// the stack; longer text uses a temporary named allocation.
BinStream& BinStream::operator>>(Symbol& symbol) {
    constexpr unsigned int kStackTextLimit = 0x3FF;
    unsigned int length = 0;
    ReadEndian(&length, sizeof(length));
    if (length > kStackTextLimit) {
        auto* text = static_cast<char*>(
            MemAlloc(length + 1, "BinStream::operator>>(Symbol)", 0));
        Read(text, length);
        text[length] = '\0';
        symbol = Symbol(text);
        MemFree(text);
        return *this;
    }
    char text[kStackTextLimit + 1];
    Read(text, length);
    text[length] = '\0';
    symbol = Symbol(text);
    return *this;
}

// Reconstructed from eboot.elf at 0x21ACB0. Byte-swapped values are staged in
// an eight-byte temporary. Sizes other than 1, 2, 4, and 8 leave that
// temporary unspecified in the original and are zero-initialized here.
void BinStream::WriteEndian(const void* data, int size) {
    const void* source = data;
    std::uint8_t swapped[8] = {};
    if (mLittleEndian != 0) {
        if (size == 1 || size == 2 || size == 4 || size == 8) {
            std::memcpy(swapped, data, static_cast<std::size_t>(size));
            SwapValue(swapped, size);
        }
        source = swapped;
    }

    Fail();
    if (mCrypto == nullptr) {
        WriteImpl(source, static_cast<unsigned long>(size));
        return;
    }
    WriteEncrypted(source, static_cast<unsigned long>(size));
}

// Reconstructed from eboot.elf at 0x21AB60.
unsigned long BinStream::ReadAsync(void* data, unsigned long size) {
    Fail();
    ReadImpl(data, size);
    Decrypt(data, size);
    return Fail() ? 0 : size;
}

// Reconstructed from eboot.elf at 0x219C30.
void BinStream::PatchSize(long position) {
    const long end = Tell();
    SeekImpl(position, kSeekBegin);
    const long size = end - position - 8;
    WriteEndian(&size, sizeof(size));
    SeekImpl(end, kSeekBegin);
}

// Reconstructed from eboot.elf at 0x219CD0.
const char* BinStream::Name() const {
    return "<unnamed>";
}

// Reconstructed from eboot.elf at 0x219CC0.
void BinStream::Seek(long offset, SeekType origin) {
    SeekImpl(offset, origin);
}
