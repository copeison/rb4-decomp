#include "core/io/bin_stream.h"

#include <cstring>

#include "core/memory/engine_memory.h"
#include "core/random/random_generator.h"

namespace rb4 {

namespace {

constexpr std::size_t kCipherChunkSize = 512;

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

void swap_value(void* data, std::int32_t size) {
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

void decrypt(BinStream& stream, void* data, std::int64_t size) {
    if (stream.cipher == nullptr || size <= 0) {
        return;
    }
    auto* bytes = static_cast<std::uint8_t*>(data);
    for (std::int64_t index = 0; index < size; ++index) {
        bytes[index] ^= static_cast<std::uint8_t>(
            random_generator_next(*stream.cipher));
    }
}

// Encrypted writes pass through a 512-byte stack buffer in chunks.
void write_encrypted(BinStream& stream, const void* data, std::size_t size) {
    std::uint8_t chunk[kCipherChunkSize];
    const auto* source = static_cast<const std::uint8_t*>(data);
    while (size != 0) {
        const auto count = size < kCipherChunkSize ? size : kCipherChunkSize;
        for (std::size_t index = 0; index < count; ++index) {
            chunk[index] = static_cast<std::uint8_t>(
                random_generator_next(*stream.cipher) ^ source[index]);
        }
        source += count;
        stream.dispatch->write_impl(
            &stream, chunk, static_cast<std::int64_t>(count));
        size -= count;
    }
}

void bin_stream_destruct_slot(BinStream* stream) {
    bin_stream_destruct(*stream);
}

// Reconstructed from eboot.elf at 0x21A6D0.
void bin_stream_delete(BinStream* stream) {
    bin_stream_destruct(*stream);
    render_release(stream);
}

// The base dispatch at 0x18EF020 leaves the stream-specific operations pure.
const BinStreamDispatch kBinStreamDispatch{
    bin_stream_destruct_slot,
    bin_stream_delete,
    nullptr,
    nullptr,
    bin_stream_default_nop,
    nullptr,
    nullptr,
    bin_stream_default_name,
    bin_stream_default_size,
    bin_stream_read_checked,
    bin_stream_default_zero,
    bin_stream_patch_size,
    nullptr,
    nullptr,
    nullptr,
};

}  // namespace

// Reconstructed from eboot.elf at 0x21A680.
void bin_stream_construct(
    BinStream& stream,
    std::uint32_t swap_endian,
    std::uint32_t platform) {
    stream.dispatch = &kBinStreamDispatch;
    stream.reserved_8 = -1;
    stream.reserved_12 = -1;
    stream.reserved_16 = false;
    stream.swap_endian = swap_endian;
    stream.cipher = nullptr;
    stream.platform = platform;
}

// Reconstructed from eboot.elf at 0x21A6B0. The stream owns its cipher.
void bin_stream_destruct(BinStream& stream) {
    stream.dispatch = &kBinStreamDispatch;
    if (stream.cipher != nullptr) {
        render_release(stream.cipher);
    }
}

// Reconstructed from eboot.elf at 0x21A280. The fail query is issued before
// every read and its result is ignored, matching the original.
void bin_stream_read(BinStream& stream, void* data, std::int64_t size) {
    stream.dispatch->fail(&stream);
    stream.dispatch->read_impl(&stream, data, size);
    decrypt(stream, data, size);
}

// Reconstructed from eboot.elf at 0x21ABF0. Byte-swapped streams reverse 2-,
// 4-, and 8-byte values after decryption.
void bin_stream_read_endian(BinStream& stream, void* data, std::int32_t size) {
    bin_stream_read(stream, data, size);
    if (stream.swap_endian != 0) {
        swap_value(data, size);
    }
}

// Reconstructed from eboot.elf at 0x21ACB0. Byte-swapped values are staged in
// an eight-byte temporary. Sizes other than 1, 2, 4, and 8 leave that
// temporary unspecified in the original and are zero-initialized here.
void bin_stream_write_endian(
    BinStream& stream,
    const void* data,
    std::int32_t size) {
    const void* source = data;
    std::uint8_t swapped[8] = {};
    if (stream.swap_endian != 0) {
        if (size == 1 || size == 2 || size == 4 || size == 8) {
            std::memcpy(swapped, data, static_cast<std::size_t>(size));
            swap_value(swapped, size);
        }
        source = swapped;
    }

    stream.dispatch->fail(&stream);
    if (stream.cipher == nullptr) {
        stream.dispatch->write_impl(&stream, source, size);
        return;
    }
    write_encrypted(stream, source, static_cast<std::size_t>(size));
}

// Reconstructed from eboot.elf at 0x21AB60. Returns the requested size, or
// zero once the stream has failed.
std::int64_t bin_stream_read_checked(
    BinStream* stream,
    void* data,
    std::int64_t size) {
    stream->dispatch->fail(stream);
    stream->dispatch->read_impl(stream, data, size);
    decrypt(*stream, data, size);
    return stream->dispatch->fail(stream) ? 0 : size;
}

// Reconstructed from eboot.elf at 0x219C30. Rewrites the 64-bit size field at
// the given position with the number of bytes that follow it.
void bin_stream_patch_size(BinStream* stream, std::int64_t position) {
    const std::int64_t end = stream->dispatch->tell(stream);
    stream->dispatch->seek_impl(stream, position, BinStreamSeek::kBegin);
    const std::int64_t size = end - position - 8;
    bin_stream_write_endian(*stream, &size, sizeof(size));
    stream->dispatch->seek_impl(stream, end, BinStreamSeek::kBegin);
}

// Reconstructed from eboot.elf at 0x219CD0.
const char* bin_stream_default_name(BinStream*) {
    return "<unnamed>";
}

// Reconstructed from eboot.elf at 0xD7990.
std::int64_t bin_stream_default_size(BinStream*) {
    return 0;
}

// Reconstructed from eboot.elf at 0xD79A0.
std::int64_t bin_stream_default_zero(BinStream*) {
    return 0;
}

// Reconstructed from eboot.elf at 0xD7960.
void bin_stream_default_nop(BinStream*) {}

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
