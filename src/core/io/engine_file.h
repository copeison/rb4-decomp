#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct EngineFile;

// Leading slots of the engine file interface. Every public engine_file_*
// helper is a thin wrapper around one of these virtual calls.
struct EngineFileDispatch {
    void* reserved_0;
    void (*close)(EngineFile* file);
    void* reserved_16;
    std::int64_t (*read)(EngineFile* file, void* data, std::int64_t size);
    void* reserved_32;
    std::int64_t (*write)(
        EngineFile* file,
        const void* data,
        std::int64_t size);
    void* reserved_48;
    std::int64_t (*seek)(
        EngineFile* file,
        std::int64_t offset,
        std::int32_t origin);
    void (*flush)(EngineFile* file);
    std::int32_t (*tell)(EngineFile* file);
    bool (*eof)(EngineFile* file);
    bool (*fail)(EngineFile* file);
    std::int64_t (*size)(EngineFile* file);
};

static_assert(offsetof(EngineFileDispatch, read) == 0x18);
static_assert(offsetof(EngineFileDispatch, write) == 0x28);
static_assert(offsetof(EngineFileDispatch, seek) == 0x38);
static_assert(offsetof(EngineFileDispatch, size) == 0x60);

struct EngineFile {
    EngineFileDispatch* dispatch;
};

EngineFile* engine_file_open(const char* path, std::uint32_t mode);
bool engine_file_failed(EngineFile* file);
void engine_file_close(EngineFile* file);
std::int64_t engine_file_read(EngineFile* file, void* data, std::int64_t size);
std::int64_t engine_file_write(
    EngineFile* file,
    const void* data,
    std::int64_t size);
std::int64_t engine_file_seek(
    EngineFile* file,
    std::int64_t offset,
    std::int32_t origin);
std::int32_t engine_file_tell(EngineFile* file);
void engine_file_flush(EngineFile* file);
bool engine_file_eof(EngineFile* file);
std::int64_t engine_file_get_size(EngineFile* file);

}  // namespace rb4
