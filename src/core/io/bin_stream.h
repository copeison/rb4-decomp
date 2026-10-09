#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct BinStream;
class Symbol;
struct RandomGenerator;

enum class BinStreamSeek : std::int32_t {
    kBegin = 0,
    kCurrent = 1,
    kEnd = 2,
};

struct BinStreamDispatch {
    void (*destruct)(BinStream* stream);
    void (*delete_stream)(BinStream* stream);
    void (*flush)(BinStream* stream);
    std::int32_t (*tell)(BinStream* stream);
    void (*reserved_32)(BinStream* stream);
    std::int32_t (*eof)(BinStream* stream);
    bool (*fail)(BinStream* stream);
    const char* (*name)(BinStream* stream);
    std::int64_t (*size)(BinStream* stream);
    std::int64_t (*read_checked)(
        BinStream* stream,
        void* data,
        std::int64_t size);
    std::int64_t (*reserved_80)(BinStream* stream);
    void (*patch_size)(BinStream* stream, std::int64_t position);
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
static_assert(offsetof(BinStreamDispatch, read_checked) == 72);
static_assert(offsetof(BinStreamDispatch, read_impl) == 96);
static_assert(offsetof(BinStreamDispatch, seek_impl) == 112);
static_assert(sizeof(BinStreamDispatch) == 120);

// Common 40-byte binary stream base.
struct BinStream {
    const BinStreamDispatch* dispatch;
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

void bin_stream_construct(
    BinStream& stream,
    std::uint32_t swap_endian,
    std::uint32_t platform);
void bin_stream_destruct(BinStream& stream);
void bin_stream_read(BinStream& stream, void* data, std::int64_t size);
void bin_stream_read_endian(BinStream& stream, void* data, std::int32_t size);
void bin_stream_read_symbol(BinStream& stream, Symbol& symbol);
void bin_stream_write_endian(
    BinStream& stream,
    const void* data,
    std::int32_t size);
std::int64_t bin_stream_read_checked(
    BinStream* stream,
    void* data,
    std::int64_t size);
void bin_stream_patch_size(BinStream* stream, std::int64_t position);
const char* bin_stream_default_name(BinStream* stream);
std::int64_t bin_stream_default_size(BinStream* stream);
std::int64_t bin_stream_default_zero(BinStream* stream);
void bin_stream_default_nop(BinStream* stream);
void bin_stream_seek(
    BinStream& stream,
    std::int64_t offset,
    BinStreamSeek origin);
std::int32_t bin_stream_tell(BinStream& stream);

}  // namespace rb4
