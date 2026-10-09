#include "core/io/file_stream.h"

#include <cstring>

#include "core/io/engine_file.h"
#include "core/io/file_stream_adapters.h"
#include "core/memory/engine_memory.h"
#include "render/resources/names/render_resource_name.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kFileStreamPlatform = 3;
constexpr std::size_t kChecksumContextOffset = 8;
constexpr std::size_t kChecksumNameOffset = 0xD8;

FileStream& file_stream(BinStream* stream) {
    return *static_cast<FileStream*>(stream);
}

// The checksum owns a SHA-1 context at +0x08 and a name string at +0xD8.
void release_checksum(FileStream& stream) {
    auto* checksum = reinterpret_cast<std::uint8_t*>(stream.checksum);
    if (checksum != nullptr) {
        render_resource_name_destruct(*reinterpret_cast<RenderResourceName*>(
            checksum + kChecksumNameOffset));
        sha1_context_reset(checksum + kChecksumContextOffset);
        render_release(checksum);
    }
    stream.checksum = nullptr;
    stream.checksum_bytes = 0;
}

void file_stream_destruct_slot(BinStream* stream) {
    file_stream_destruct(file_stream(stream));
}

// Reconstructed from eboot.elf at 0x244580.
void file_stream_delete(BinStream* stream) {
    file_stream_destruct(file_stream(stream));
    render_release(stream);
}

// Reconstructed from eboot.elf at 0x2446A0.
void file_stream_flush(BinStream* stream) {
    engine_file_flush(file_stream(stream).file);
}

// Reconstructed from eboot.elf at 0x2446E0.
std::int32_t file_stream_tell(BinStream* stream) {
    return engine_file_tell(file_stream(stream).file);
}

// Reconstructed from eboot.elf at 0x2446F0.
std::int32_t file_stream_eof(BinStream* stream) {
    return engine_file_eof(file_stream(stream).file) ? 1 : 0;
}

// Reconstructed from eboot.elf at 0x244800.
const char* file_stream_name(BinStream* stream) {
    return file_stream(stream).name;
}

// Reconstructed from eboot.elf at 0x244610.
void file_stream_read_impl(BinStream* stream, void* data, std::int64_t size) {
    auto& file = file_stream(stream);
    if (engine_file_read(file.file, data, size) != size) {
        file.failed = true;
        return;
    }
    if (file.checksum != nullptr) {
        stream_checksum_update(file.checksum, data, size);
        file.checksum_bytes += size;
    }
}

// Reconstructed from eboot.elf at 0x244670.
void file_stream_write_impl(
    BinStream* stream,
    const void* data,
    std::int64_t size) {
    auto& file = file_stream(stream);
    if (engine_file_write(file.file, data, size) != size) {
        file.failed = true;
    }
}

// Reconstructed from eboot.elf at 0x2446B0.
void file_stream_seek_impl(
    BinStream* stream,
    std::int64_t offset,
    BinStreamSeek origin) {
    auto& file = file_stream(stream);
    if (engine_file_seek(file.file, offset, static_cast<std::int32_t>(origin)) <
        0) {
        file.failed = true;
    }
}

// The file-stream dispatch at 0x18EF280 inherits slots 4 and 9-11 from the
// common binary stream.
const BinStreamDispatch kFileStreamDispatch{
    file_stream_destruct_slot,
    file_stream_delete,
    file_stream_flush,
    file_stream_tell,
    bin_stream_default_nop,
    file_stream_eof,
    file_stream_fail,
    file_stream_name,
    file_stream_size,
    bin_stream_read_checked,
    bin_stream_default_zero,
    bin_stream_patch_size,
    file_stream_read_impl,
    file_stream_write_impl,
    file_stream_seek_impl,
};

}  // namespace

// Reconstructed from eboot.elf at 0x2443A0. The name is copied with strncpy
// semantics, and the size is queried only after a successful open.
void file_stream_construct(
    FileStream& stream,
    const char* path,
    std::uint32_t mode,
    std::uint32_t swap_endian) {
    bin_stream_construct(stream, swap_endian, kFileStreamPlatform);
    stream.dispatch = &kFileStreamDispatch;
    stream.size = 0;
    stream.checksum = nullptr;
    stream.checksum_bytes = 0;
    std::strncpy(stream.name, path, sizeof(stream.name));
    stream.file = engine_file_open(path, mode);
    stream.failed = engine_file_failed(stream.file);
    if (!stream.failed) {
        stream.size = engine_file_get_size(stream.file);
    }
}

// Reconstructed from eboot.elf at 0x2444A0. Only named streams whose file
// opened successfully close it.
void file_stream_destruct(FileStream& stream) {
    stream.dispatch = &kFileStreamDispatch;
    if (stream.name[0] != '\0' && !engine_file_failed(stream.file)) {
        engine_file_close(stream.file);
    }
    release_checksum(stream);
    bin_stream_destruct(stream);
}

// Reconstructed from eboot.elf at 0x244710.
bool file_stream_fail(BinStream* stream) {
    return file_stream(stream).failed;
}

// Reconstructed from eboot.elf at 0x244720.
std::int64_t file_stream_size(BinStream* stream) {
    return file_stream(stream).size;
}

}  // namespace rb4
