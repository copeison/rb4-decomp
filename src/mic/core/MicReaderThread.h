#pragma once

#include <cstddef>

#include "utl/threading/Thread.h"

class MicHwManager;

// Thread that polls the microphones (mic/MicReaderThread.o). The map's
// thread serves one Mic_FMOD; in this build a single instance polls every
// mic of the MicHwManager. The object is 152 bytes. Field names are not in
// the reference map.
class MicReaderThread {
public:
    explicit MicReaderThread(MicHwManager* manager);  // 0xA2BD0
    ~MicReaderThread();                               // 0xA2C50

    // Creates and starts the instance. At 0xA29E0. The map has
    // Init(MicHwManager_FMOD*, Mic_FMOD*, int).
    static void Init(MicHwManager* manager);
    // Stops and deletes the instance. At 0xA2B80.
    static void Destroy();
    // Starts the "mic_reader" thread. At 0xA2AF0.
    void StartAsyncPoll();
    // Asks the thread to stop and joins it. At 0xA2C80.
    void QuitAsyncPoll();
    // The thread entry at 0xA2C90.
    static int _MicThreadMain(void* context);
    // The polling loop the entry runs: mic polls every 11 ms. At 0xA2D00.
    // Name not in the reference map.
    void _PollLoop();

    NamedThread mThread;
    bool mQuit;
    MicHwManager* mManager;

    static MicReaderThread* sInstance;  // 0x19C9040. Name not in the reference map.
};

static_assert(offsetof(MicReaderThread, mQuit) == 136);
static_assert(offsetof(MicReaderThread, mManager) == 144);
static_assert(sizeof(MicReaderThread) == 152);
