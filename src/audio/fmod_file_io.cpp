#include "fmod_file_io.h"

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace rb4 {

struct EngineFile;

EngineFile* engine_file_open(const char* path, std::uint32_t mode);
void engine_file_prepare_for_reading(EngineFile* file);
void engine_file_close(EngineFile* file);
std::uint32_t engine_file_read(
    EngineFile* file, void* buffer, std::uint32_t size);
void engine_file_seek(
    EngineFile* file, std::uint32_t position, std::int32_t origin);
std::uint32_t engine_file_get_size(EngineFile* file);

namespace {

constexpr std::uint32_t kEngineFileReadMode = 2;
constexpr std::int32_t kSeekFromStart = 0;

struct FmodFileHandle {
    std::string path;
    std::recursive_mutex mutex;
    EngineFile* file = nullptr;
};

std::mutex g_async_mutex;
std::condition_variable g_async_changed;
std::vector<FMOD_ASYNCREADINFO*> g_async_requests;
FMOD_ASYNCREADINFO* g_active_request = nullptr;
bool g_async_stopping = false;
std::thread g_async_thread;

FmodFileHandle* as_file_handle(void* handle) {
    return static_cast<FmodFileHandle*>(handle);
}

void process_async_request(FMOD_ASYNCREADINFO& request) {
    auto result = fmod_file_seek(request.handle, request.offset, nullptr);
    if (result == FMOD_OK) {
        result = fmod_file_read(
            request.handle,
            request.buffer,
            request.sizebytes,
            &request.bytesread,
            nullptr);
    }
    if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF) {
        result = FMOD_ERR_FILE_BAD;
    }
    request.done(&request, result);
}

// Reconstructed from eboot.elf at 0x27A270.
void fmod_async_file_reader_thread() {
    std::unique_lock<std::mutex> lock(g_async_mutex);
    while (!g_async_stopping) {
        g_async_changed.wait(lock, [] {
            return g_async_stopping || !g_async_requests.empty();
        });
        if (g_async_stopping) {
            break;
        }

        g_active_request = g_async_requests.back();
        g_async_requests.pop_back();

        lock.unlock();
        process_async_request(*g_active_request);
        lock.lock();

        g_active_request = nullptr;
        g_async_changed.notify_all();
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x27A5A0.
FMOD_RESULT fmod_file_open(
    const char* name,
    std::uint32_t* file_size,
    void** handle,
    void* user_data) {
    (void)user_data;

    auto* file_handle = new FmodFileHandle;
    file_handle->path = name != nullptr ? name : "";

    {
        const std::lock_guard<std::recursive_mutex> lock(file_handle->mutex);
        file_handle->file = engine_file_open(name, kEngineFileReadMode);
        if (file_handle->file != nullptr) {
            engine_file_prepare_for_reading(file_handle->file);
            *file_size = engine_file_get_size(file_handle->file);
        }
    }

    if (file_handle->file == nullptr) {
        delete file_handle;
        return FMOD_ERR_FILE_NOTFOUND;
    }

    *handle = file_handle;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A6D0.
FMOD_RESULT fmod_file_close(void* handle, void* user_data) {
    (void)user_data;
    auto* file_handle = as_file_handle(handle);
    if (file_handle == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }

    {
        const std::lock_guard<std::recursive_mutex> lock(file_handle->mutex);
        engine_file_close(file_handle->file);
        file_handle->file = nullptr;
    }
    delete file_handle;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A530.
FMOD_RESULT fmod_file_read(
    void* handle,
    void* buffer,
    std::uint32_t size,
    std::uint32_t* bytes_read,
    void* user_data) {
    (void)user_data;
    auto* file_handle = as_file_handle(handle);
    if (file_handle == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }

    const std::lock_guard<std::recursive_mutex> lock(file_handle->mutex);
    *bytes_read = engine_file_read(file_handle->file, buffer, size);
    return *bytes_read < size ? FMOD_ERR_FILE_EOF : FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A4D0.
FMOD_RESULT fmod_file_seek(
    void* handle,
    std::uint32_t position,
    void* user_data) {
    (void)user_data;
    auto* file_handle = as_file_handle(handle);
    if (file_handle == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }

    const std::lock_guard<std::recursive_mutex> lock(file_handle->mutex);
    engine_file_seek(file_handle->file, position, kSeekFromStart);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A780.
FMOD_RESULT fmod_file_async_read(FMOD_ASYNCREADINFO* info, void* user_data) {
    (void)user_data;
    if (info == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }

    {
        const std::lock_guard<std::mutex> lock(g_async_mutex);
        const auto position = std::lower_bound(
            g_async_requests.begin(),
            g_async_requests.end(),
            info->priority,
            [](const FMOD_ASYNCREADINFO* queued, std::int32_t priority) {
                return queued->priority < priority;
            });
        g_async_requests.insert(position, info);
    }
    g_async_changed.notify_one();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A850.
FMOD_RESULT fmod_file_async_cancel(FMOD_ASYNCREADINFO* info, void* user_data) {
    (void)user_data;
    if (info == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(g_async_mutex);
    const auto position =
        std::find(g_async_requests.begin(), g_async_requests.end(), info);
    if (position != g_async_requests.end()) {
        g_async_requests.erase(position);
    } else {
        g_async_changed.wait(lock, [info] { return g_active_request != info; });
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A1C0.
void fmod_async_file_reader_initialize() {
    g_async_stopping = false;
    g_async_thread = std::thread(fmod_async_file_reader_thread);
}

// Reconstructed from eboot.elf at 0x27A460.
void fmod_async_file_reader_shutdown() {
    {
        const std::lock_guard<std::mutex> lock(g_async_mutex);
        g_async_stopping = true;
    }
    g_async_changed.notify_one();
    if (g_async_thread.joinable()) {
        g_async_thread.join();
    }
}

}  // namespace rb4
