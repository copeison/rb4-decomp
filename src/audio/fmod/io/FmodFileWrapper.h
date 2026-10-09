#pragma once

#include <cstddef>

#include "audio/core/system/Audio.h"
#include "audio/fmod/api/fmod_api.h"
#include "utl/text/Str.h"
#include "os/threading/CritSec.h"

class File;

// Engine file opened for FMOD. FMOD's file callbacks receive it as their
// handle, and the async reader thread, named "FmodFileWrapper", serves its
// queued reads. The callbacks are at 0x279FE0 through 0x27A920. Names not in
// the reference map.
class FmodFileWrapper {
public:
    // Reconstructed from eboot.elf at 0x279FE0.
    FMOD_RESULT Open(const char* path, unsigned int* fileSize);

    String mPath;
    CritSec mCritSec;
    File* mFile;
};

static_assert(offsetof(FmodFileWrapper, mCritSec) == 16);
static_assert(offsetof(FmodFileWrapper, mFile) == 32);
static_assert(sizeof(FmodFileWrapper) == 40);

// FMOD file-system callbacks. Names not in the reference map.
FMOD_RESULT FmodFileOpen(const char* name, unsigned int* fileSize, void** handle, void* userData);
FMOD_RESULT FmodFileClose(void* handle, void* userData);
FMOD_RESULT FmodFileRead(
    void* handle, void* buffer, unsigned int size, unsigned int* bytesRead, void* userData);
FMOD_RESULT FmodFileSeek(void* handle, unsigned int position, void* userData);
FMOD_RESULT FmodFileAsyncRead(FMOD_ASYNCREADINFO* info, void* userData);
FMOD_RESULT FmodFileAsyncCancel(FMOD_ASYNCREADINFO* info, void* userData);

// Starts and stops the async reader thread. At 0x27A1C0 and 0x27A460. Names
// not in the reference map.
void FmodFileWrapperStartReader();
void FmodFileWrapperStopReader();
