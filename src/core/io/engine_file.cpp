#include "core/io/engine_file.h"

#include "core/io/engine_file_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x378940.
EngineFile* engine_file_open(const char* path, std::uint32_t mode) {
    return engine_file_system_open(path, mode);
}

// Reconstructed from eboot.elf at 0x378950. A missing file reports failure.
bool engine_file_failed(EngineFile* file) {
    if (file == nullptr) {
        return true;
    }
    return file->dispatch->fail(file);
}

// Reconstructed from eboot.elf at 0x378960.
void engine_file_close(EngineFile* file) {
    if (file != nullptr) {
        file->dispatch->close(file);
    }
}

// Reconstructed from eboot.elf at 0x378A10.
std::int64_t engine_file_read(EngineFile* file, void* data, std::int64_t size) {
    return file->dispatch->read(file, data, size);
}

// Reconstructed from eboot.elf at 0x378A40.
std::int64_t engine_file_write(
    EngineFile* file,
    const void* data,
    std::int64_t size) {
    return file->dispatch->write(file, data, size);
}

// Reconstructed from eboot.elf at 0x378A50.
std::int64_t engine_file_seek(
    EngineFile* file,
    std::int64_t offset,
    std::int32_t origin) {
    return file->dispatch->seek(file, offset, origin);
}

// Reconstructed from eboot.elf at 0x378A60.
std::int32_t engine_file_tell(EngineFile* file) {
    return file->dispatch->tell(file);
}

// Reconstructed from eboot.elf at 0x378A70.
void engine_file_flush(EngineFile* file) {
    file->dispatch->flush(file);
}

// Reconstructed from eboot.elf at 0x378A80.
bool engine_file_eof(EngineFile* file) {
    return file->dispatch->eof(file);
}

// Reconstructed from eboot.elf at 0x378A90.
std::int64_t engine_file_get_size(EngineFile* file) {
    return file->dispatch->size(file);
}

}  // namespace rb4
