#pragma once

#include <cstddef>
#include <cstdint>

#include "core/io/bin_stream.h"

namespace rb4 {

struct EngineFile;
struct StreamChecksum;

// 592-byte binary stream over an engine file. The optional checksum is
// attached by its owner and updated with every successful read.
struct FileStream : BinStream {
    EngineFile* file;
    char name[512];
    bool failed;
    std::uint8_t reserved_561[7];
    std::int64_t size;
    StreamChecksum* checksum;
    std::int64_t checksum_bytes;
};

static_assert(offsetof(FileStream, file) == 40);
static_assert(offsetof(FileStream, name) == 48);
static_assert(offsetof(FileStream, failed) == 560);
static_assert(offsetof(FileStream, size) == 568);
static_assert(offsetof(FileStream, checksum) == 576);
static_assert(sizeof(FileStream) == 592);

void file_stream_construct(
    FileStream& stream,
    const char* path,
    std::uint32_t mode,
    std::uint32_t swap_endian);
void file_stream_destruct(FileStream& stream);
bool file_stream_fail(BinStream* stream);
std::int64_t file_stream_size(BinStream* stream);

}  // namespace rb4
