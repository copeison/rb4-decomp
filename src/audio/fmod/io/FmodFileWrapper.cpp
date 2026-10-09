#include "audio/fmod/io/FmodFileWrapper.h"

#include <cstring>

#include "os/files/File.h"
#include "utl/containers/Std.h"
#include "utl/threading/Thread.h"

namespace {

// Engine file mode used for every FMOD file. Name not in the reference map.
constexpr FileMode kFmodFileMode = static_cast<FileMode>(2);

// Calls File slot 17, which prepares a freshly opened file for streaming
// reads. os/files/File.h does not declare slots past 12 yet.
void PrepareForStreaming(File* file) {
    using Slot = void (*)(File*);
    (*reinterpret_cast<Slot* const*>(file))[17](file);
}

// The async reader's state. The globals are at 0x19F2F78 through 0x19F3050.
// Names not in the reference map.
struct ReadQueue {
    FMOD_ASYNCREADINFO** mBegin;
    FMOD_ASYNCREADINFO** mEnd;
    FMOD_ASYNCREADINFO** mCapacity;
    void* mAllocator;
};

NamedThread gReaderThread;
ScePthreadMutex* gReaderConditionMutex;
ScePthreadCond gReaderCondition;
CritSec gReaderCritSec;
ReadQueue gReadQueue;
bool gReaderStopping;
FMOD_ASYNCREADINFO* gActiveRead;

// Inserts a request before the first queued request of equal or higher
// priority, so the highest priority sits at the back. Reconstructed from the
// EASTL insert at 0x27A920.
void InsertRead(FMOD_ASYNCREADINFO** position, FMOD_ASYNCREADINFO* info) {
    if (gReadQueue.mEnd == gReadQueue.mCapacity) {
        const long count = gReadQueue.mEnd - gReadQueue.mBegin;
        const long offset = position - gReadQueue.mBegin;
        const long capacity = count != 0 ? count * 2 : 1;
        auto** storage = static_cast<FMOD_ASYNCREADINFO**>(
            HmxAllocator::gStlAllocator.allocate(capacity * sizeof(FMOD_ASYNCREADINFO*)));
        std::memmove(storage, gReadQueue.mBegin, offset * sizeof(*storage));
        storage[offset] = info;
        std::memmove(storage + offset + 1, position, (count - offset) * sizeof(*storage));
        if (gReadQueue.mBegin != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(
                gReadQueue.mBegin,
                (gReadQueue.mCapacity - gReadQueue.mBegin) * sizeof(*storage));
        }
        gReadQueue.mBegin = storage;
        gReadQueue.mEnd = storage + count + 1;
        gReadQueue.mCapacity = storage + capacity;
        return;
    }
    std::memmove(position + 1, position, (gReadQueue.mEnd - position) * sizeof(*position));
    *position = info;
    ++gReadQueue.mEnd;
}

// Reconstructed from eboot.elf at 0x27A270.
int ReaderThread(void*) {
    gReaderCritSec.Enter();
    while (!gReaderStopping) {
        if (gReadQueue.mBegin != gReadQueue.mEnd) {
            gActiveRead = *--gReadQueue.mEnd;
        }
        if (gActiveRead == nullptr) {
            scePthreadCondWait(&gReaderCondition, gReaderConditionMutex);
            continue;
        }

        gReaderCritSec.Exit();
        FMOD_ASYNCREADINFO* info = gActiveRead;
        FMOD_RESULT result = FMOD_ERR_INVALID_PARAM;
        auto* wrapper = static_cast<FmodFileWrapper*>(info->handle);
        if (wrapper != nullptr) {
            wrapper->mCritSec.Enter();
            FileSeek(wrapper->mFile, info->offset, kSeekBegin);
            wrapper->mCritSec.Exit();
            const unsigned int size = info->sizebytes;
            wrapper->mCritSec.Enter();
            info->bytesread = FileRead(wrapper->mFile, info->buffer, size);
            result = info->bytesread < size ? FMOD_ERR_FILE_EOF : FMOD_OK;
            wrapper->mCritSec.Exit();
        }
        if (result != FMOD_OK && result != FMOD_ERR_FILE_EOF) {
            result = FMOD_ERR_FILE_BAD;
        }
        info->done(info, result);

        gReaderCritSec.Enter();
        gActiveRead = nullptr;
        scePthreadCondSignal(&gReaderCondition);
    }
    gReaderCritSec.Exit();
    return 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x279FE0.
FMOD_RESULT FmodFileWrapper::Open(const char* path, unsigned int* fileSize) {
    CritSecTracker tracker(&mCritSec);
    if (path != nullptr) {
        mPath += path;
    }
    mFile = static_cast<File*>(FileOpen(path, kFmodFileMode));
    if (mFile == nullptr) {
        return FMOD_ERR_FILE_NOTFOUND;
    }
    reinterpret_cast<void**>(mFile)[1] = nullptr;
    PrepareForStreaming(mFile);
    *fileSize = static_cast<unsigned int>(FileSize(mFile));
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A5A0.
FMOD_RESULT FmodFileOpen(const char* name, unsigned int* fileSize, void** handle, void*) {
    auto* wrapper = new FmodFileWrapper();
    const FMOD_RESULT result = wrapper->Open(name, fileSize);
    if (result != FMOD_OK) {
        delete wrapper;
        return result;
    }
    *handle = wrapper;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A6D0.
FMOD_RESULT FmodFileClose(void* handle, void*) {
    auto* wrapper = static_cast<FmodFileWrapper*>(handle);
    if (wrapper == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }
    wrapper->mCritSec.Enter();
    FileClose(wrapper->mFile);
    wrapper->mCritSec.Exit();
    delete wrapper;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A530.
FMOD_RESULT FmodFileRead(
    void* handle, void* buffer, unsigned int size, unsigned int* bytesRead, void*) {
    auto* wrapper = static_cast<FmodFileWrapper*>(handle);
    if (wrapper == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }
    CritSecTracker tracker(&wrapper->mCritSec);
    *bytesRead = static_cast<unsigned int>(FileRead(wrapper->mFile, buffer, size));
    return *bytesRead < size ? FMOD_ERR_FILE_EOF : FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A4D0.
FMOD_RESULT FmodFileSeek(void* handle, unsigned int position, void*) {
    auto* wrapper = static_cast<FmodFileWrapper*>(handle);
    if (wrapper == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }
    CritSecTracker tracker(&wrapper->mCritSec);
    FileSeek(wrapper->mFile, position, kSeekBegin);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A780.
FMOD_RESULT FmodFileAsyncRead(FMOD_ASYNCREADINFO* info, void*) {
    if (info == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }
    CritSecTracker tracker(&gReaderCritSec);
    auto** position = gReadQueue.mBegin;
    while (position != gReadQueue.mEnd && (*position)->priority < info->priority) {
        ++position;
    }
    if (position != gReadQueue.mEnd || gReadQueue.mEnd == gReadQueue.mCapacity) {
        InsertRead(position, info);
    } else {
        *gReadQueue.mEnd++ = info;
    }
    scePthreadCondSignal(&gReaderCondition);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A850. A queued request is removed; an
// active one is waited out.
FMOD_RESULT FmodFileAsyncCancel(FMOD_ASYNCREADINFO* info, void*) {
    if (info == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }
    CritSecTracker tracker(&gReaderCritSec);
    for (auto** position = gReadQueue.mBegin; position != gReadQueue.mEnd; ++position) {
        if (*position == info) {
            std::memmove(
                position, position + 1, (gReadQueue.mEnd - position - 1) * sizeof(*position));
            --gReadQueue.mEnd;
            return FMOD_OK;
        }
    }
    while (gActiveRead == info) {
        scePthreadCondWait(&gReaderCondition, gReaderConditionMutex);
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27A1C0.
void FmodFileWrapperStartReader() {
    gReaderConditionMutex = &gReaderCritSec.mCritSec;
    ScePthreadCondattr attributes;
    scePthreadCondattrInit(&attributes);
    scePthreadCondInit(&gReaderCondition, &attributes, "Condition");
    const auto& task = *ThreadMap::GetTaskSettings("stream_reader");
    gReaderThread.Create(
        ReaderThread,
        nullptr,
        "FmodFileWrapper",
        task.mProcessor,
        task.mPriority,
        task.mStackSize,
        task.mAffinityMask);
    gReaderThread.mThread.Start();
}

// Reconstructed from eboot.elf at 0x27A460.
void FmodFileWrapperStopReader() {
    gReaderCritSec.Enter();
    gReaderStopping = true;
    scePthreadCondSignal(&gReaderCondition);
    gReaderCritSec.Exit();
    gReaderThread.mThread._Join();
    if (gReaderConditionMutex != nullptr) {
        scePthreadCondDestroy(&gReaderCondition);
        gReaderConditionMutex = nullptr;
    }
}
