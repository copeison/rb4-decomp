#include "core/io/bin_stream.h"

#include <cstring>

#include "core/random/random_generator.h"

namespace rb4 {

namespace {

template <typename Value>
void swap_in_place(void* data) {
    Value value;
    std::memcpy(&value, data, sizeof(value));
    Value swapped = 0;
    for (std::size_t index = 0; index < sizeof(Value); ++index) {
        swapped = static_cast<Value>(
            (swapped << 8) | ((value >> (index * 8)) & 0xFF));
    }
    std::memcpy(data, &swapped, sizeof(swapped));
}

}  // namespace

// Reconstructed from eboot.elf at 0x21ABF0. The fail query is issued before
// every read and its result is ignored, matching the original. Encrypted
// streams XOR every byte with the next generator value; byte-swapped streams
// then reverse 2-, 4-, and 8-byte values in place.
void bin_stream_read_endian(BinStream& stream, void* data, std::int32_t size) {
    stream.dispatch->fail(&stream);
    stream.dispatch->read_impl(&stream, data, size);

    auto* bytes = static_cast<std::uint8_t*>(data);
    if (stream.cipher != nullptr && size > 0) {
        for (std::int32_t index = 0; index < size; ++index) {
            bytes[index] ^= static_cast<std::uint8_t>(
                random_generator_next(*stream.cipher));
        }
    }

    if (stream.swap_endian != 0) {
        switch (size) {
        case 8:
            swap_in_place<std::uint64_t>(data);
            break;
        case 4:
            swap_in_place<std::uint32_t>(data);
            break;
        case 2:
            swap_in_place<std::uint16_t>(data);
            break;
        default:
            break;
        }
    }
}

// Reconstructed from eboot.elf at 0x219CC0.
void bin_stream_seek(
    BinStream& stream,
    std::int64_t offset,
    BinStreamSeek origin) {
    stream.dispatch->seek_impl(&stream, offset, origin);
}

std::int32_t bin_stream_tell(BinStream& stream) {
    return stream.dispatch->tell(&stream);
}

}  // namespace rb4
