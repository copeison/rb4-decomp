#pragma once

#include <cstddef>

#include "utl/threading/Thread.h"

// Background thread that services stream readers. Only the members the FMOD
// buffered-stream manager calls are declared; the class has not been
// reconstructed.
class StreamReaderThread {
public:
    StreamReaderThread();
    ~StreamReaderThread();  // 0x264260

    // Starts the "stream_reader" thread. At 0x2643D0.
    void StartAsyncPoll();
    // Signals the thread to quit and joins it. At 0x264390.
    void QuitAsyncPoll();

    // Field names are not in the reference map.
    bool mQuit;
    NamedThread mThread;
    unsigned char mUnknown144[44];
    unsigned char mSemaphore[16];  // sem_t
};

static_assert(offsetof(StreamReaderThread, mThread) == 8);
static_assert(offsetof(StreamReaderThread, mSemaphore) == 188);
