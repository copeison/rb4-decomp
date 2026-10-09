#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct BinStream;
struct RandomGenerator;

enum class BinStreamSeek : std::uint32_t {
    kBegin = 0,
    kCurrent = 1,
    kEnd = 2,
};

struct BinStreamDispatch {
    void (*destruct)(BinStream* stream);
    void (*delete_stream)(BinStream* stream);
    void (*flush)(BinStream* stream);
    std::int32_t (*tell)(BinStream* stream);
    void* reserved_32;
    void* reserved_40;
    bool (*fail)(BinStream* stream);
    void* reserved_56[5];
    void (*read_impl)(BinStream* stream, void* data, std::int64_t size);
    void (*write_impl)(
        BinStream* stream,
        const void* data,
        std::int64_t size);
    void (*seek_impl)(
        BinStream* stream,
        std::int64_t offset,
        BinStreamSeek origin);
};

static_assert(offsetof(BinStreamDispatch, tell) == 24);
static_assert(offsetof(BinStreamDispatch, fail) == 48);
static_assert(offsetof(BinStreamDispatch, read_impl) == 96);
static_assert(offsetof(BinStreamDispatch, seek_impl) == 112);

// Common 40-byte binary stream base. Construction at 0x21A680 sets both
// leading words to -1 and records the byte-swap flag and platform.
struct BinStream {
    BinStreamDispatch* dispatch;
    std::int32_t reserved_8;
    std::int32_t reserved_12;
    bool reserved_16;
    std::uint8_t reserved_17[3];
    std::uint32_t swap_endian;
    RandomGenerator* cipher;
    std::uint32_t platform;
    std::uint8_t reserved_36[4];
};

static_assert(offsetof(BinStream, swap_endian) == 20);
static_assert(offsetof(BinStream, cipher) == 24);
static_assert(offsetof(BinStream, platform) == 32);
static_assert(sizeof(BinStream) == 40);

void bin_stream_read_endian(BinStream& stream, void* data, std::int32_t size);
void bin_stream_seek(
    BinStream& stream,
    std::int64_t offset,
    BinStreamSeek origin);
std::int32_t bin_stream_tell(BinStream& stream);

}  // namespace rb4
