#include "mic/core/MicReaderThread.h"

#include <unistd.h>

#include "mic/core/MicHwManager.h"

namespace {

// Passes between the platforms' connection checks, and the pause after each
// pass. Names not in the reference map.
constexpr int kPollsPerConnectionCheck = 100;
constexpr unsigned int kPollIntervalUs = 11000;

}  // namespace

MicReaderThread* MicReaderThread::sInstance = nullptr;

// Reconstructed from eboot.elf at 0xA29E0. The constructor and StartAsyncPoll
// are inlined.
void MicReaderThread::Init(MicHwManager* manager) {
    sInstance = new MicReaderThread(manager);
    sInstance->StartAsyncPoll();
}

// Reconstructed from eboot.elf at 0xA2AF0.
void MicReaderThread::StartAsyncPoll() {
    const auto& task = *ThreadMap::GetTaskSettings("mic_reader");
    mThread.Create(
        _MicThreadMain,
        this,
        "mic_reader",
        task.mProcessor,
        task.mPriority,
        task.mStackSize,
        task.mAffinityMask);
    mThread.mThread.Start();
}

// Reconstructed from eboot.elf at 0xA2B80.
void MicReaderThread::Destroy() {
    MicReaderThread* thread = sInstance;
    if (thread != nullptr) {
        sInstance = nullptr;
        delete thread;
    }
}

// Reconstructed from eboot.elf at 0xA2BD0.
MicReaderThread::MicReaderThread(MicHwManager* manager) : mQuit(false), mManager(manager) {
    mThread.Init("Unknown Thread!");
}

// Reconstructed from eboot.elf at 0xA2C50.
MicReaderThread::~MicReaderThread() {
    mQuit = true;
    mThread.mThread._Join();
    mThread.mThread._ForceKillThread();
}

// Reconstructed from eboot.elf at 0xA2C80.
void MicReaderThread::QuitAsyncPoll() {
    mQuit = true;
    mThread.mThread._Join();
}

// Reconstructed from eboot.elf at 0xA2C90.
int MicReaderThread::_MicThreadMain(void* context) {
    static_cast<MicReaderThread*>(context)->_PollLoop();
    return 0;
}

// Reconstructed from eboot.elf at 0xA2D00.
void MicReaderThread::_PollLoop() {
    int countdown = kPollsPerConnectionCheck;
    while (!mQuit) {
        if (countdown > 1) {
            --countdown;
        } else {
            mManager->_CheckConnectsAndDisconnects();
            countdown = kPollsPerConnectionCheck;
        }
        mManager->MicThreadPoll();
        usleep(kPollIntervalUs);
    }
}
