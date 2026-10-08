#pragma once

#include <cstdint>

#include "audio/fmod/api/fmod_api.h"

namespace rb4 {

FMOD_RESULT fmod_file_open(
    const char* name,
    std::uint32_t* file_size,
    void** handle,
    void* user_data);
FMOD_RESULT fmod_file_close(void* handle, void* user_data);
FMOD_RESULT fmod_file_read(
    void* handle,
    void* buffer,
    std::uint32_t size,
    std::uint32_t* bytes_read,
    void* user_data);
FMOD_RESULT fmod_file_seek(
    void* handle,
    std::uint32_t position,
    void* user_data);
FMOD_RESULT fmod_file_async_read(FMOD_ASYNCREADINFO* info, void* user_data);
FMOD_RESULT fmod_file_async_cancel(FMOD_ASYNCREADINFO* info, void* user_data);

void fmod_async_file_reader_initialize();
void fmod_async_file_reader_shutdown();

}  // namespace rb4
