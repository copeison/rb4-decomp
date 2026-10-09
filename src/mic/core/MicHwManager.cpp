#include "mic/core/MicHwManager.h"

#include "mic/core/Mic.h"
#include "mic/core/MicHwPlatform.h"
#include "mic/core/MicReaderThread.h"

// Constructed by the static initializer at 0xA28D0, which registers the
// destructor at 0xA1C20 with atexit.
MicHwManager gMicHwManager;

// Reconstructed from eboot.elf at 0xA1C30.
void MicHwManager::Init() {
    if (!mInitialized) {
        mInitialized = true;
    }
}

// Reconstructed from eboot.elf at 0xA1C40.
void MicHwManager::_StartMicReaderThread() {
    if (MicReaderThread::sInstance == nullptr) {
        MicReaderThread::Init(this);
    }
}

// Reconstructed from eboot.elf at 0xA1C60.
MicHwManager* MicHwManager::GetInstance() {
    return &gMicHwManager;
}

// Reconstructed from eboot.elf at 0xA1C70.
void MicHwManager::Poll() {
    if (mMicsChanged) {
        mCritSec.Enter();
        _NotifyMicsChanged();
        mMicsChanged = false;
        mCritSec.Exit();
    }
    for (MicHwPlatform* platform : mPlatforms) {
        platform->Poll();
    }
    mCritSec.Enter();
    for (Mic* mic : mMics) {
        mic->Poll();
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA1D80.
void MicHwManager::_NotifyMicsChanged() {
    mCritSec.Enter();
    for (MicListener* listener : mListeners) {
        listener->OnMicsChanged();
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA1DF0.
void MicHwManager::Terminate() {
    if (!mInitialized) {
        return;
    }
    MicReaderThread::Destroy();
    for (MicHwPlatform* platform : mPlatforms) {
        platform->Terminate();
    }
    mCritSec.Enter();
    for (Mic* mic : mMics) {
        delete mic;
    }
    mMics.clear();
    mInitialized = false;
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA1EB0.
void MicHwManager::RegisterPlatform(MicHwPlatform* platform) {
    mPlatforms.push_back(platform);
}

// Reconstructed from eboot.elf at 0xA1F70.
void MicHwManager::AddMic(Mic* mic) {
    mCritSec.Enter();
    mMics.push_back(mic);
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA2060.
int MicHwManager::GetNumConnectedMics() {
    mCritSec.Enter();
    int numConnected = 0;
    for (Mic* mic : mMics) {
        if (mic->GetStatus() != 0) {
            ++numConnected;
        }
    }
    mCritSec.Exit();
    return numConnected;
}

// Reconstructed from eboot.elf at 0xA20E0.
bool MicHwManager::IsMicConnected(int index) {
    mCritSec.Enter();
    bool connected = false;
    if (index >= 0 && index < static_cast<int>(mMics.size())) {
        connected = mMics[index]->GetStatus() != 0;
    }
    mCritSec.Exit();
    return connected;
}

// Reconstructed from eboot.elf at 0xA2160.
Mic* MicHwManager::GetMic(int index) {
    mCritSec.Enter();
    Mic* mic = nullptr;
    if (index >= 0 && index < static_cast<int>(mMics.size())) {
        mic = mMics[index];
    }
    mCritSec.Exit();
    return mic;
}

// Reconstructed from eboot.elf at 0xA21D0. Returns -1 when no connected,
// unclaimed mic of the type exists.
int MicHwManager::GetNextAvailableMicID(int type) {
    mCritSec.Enter();
    int result = -1;
    for (int index = 0; index < static_cast<int>(mMics.size()); ++index) {
        int micType = mMics[index]->GetType();
        if ((type == kAnyMicType || micType == type) && !mMics[index]->mInUse &&
            mMics[index]->GetStatus() != 0) {
            result = index;
            break;
        }
    }
    mCritSec.Exit();
    return result;
}

// Reconstructed from eboot.elf at 0xA22E0.
bool MicHwManager::CaptureMic(int index, bool force) {
    mCritSec.Enter();
    bool captured = false;
    if (force || (index >= 0 && index <= static_cast<int>(mMics.size()) && !mMics[index]->mInUse)) {
        mMics[index]->mInUse = true;
        captured = true;
    }
    mCritSec.Exit();
    return captured;
}

// Reconstructed from eboot.elf at 0xA2370.
bool MicHwManager::ReleaseMic(int index, bool force) {
    mCritSec.Enter();
    bool released = false;
    if (force || (index >= 0 && index <= static_cast<int>(mMics.size()))) {
        Mic* mic = mMics[index];
        if (mic->mInUse) {
            released = true;
            mic->mInUse = false;
        }
    }
    mCritSec.Exit();
    return released;
}

// Reconstructed from eboot.elf at 0xA2400.
void MicHwManager::ReleaseAllMics() {
    mCritSec.Enter();
    for (Mic* mic : mMics) {
        mic->mInUse = false;
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA2460. The search for an existing entry
// fed a check the release build strips; the listener is always appended.
void MicHwManager::AddListener(MicListener* listener) {
    mCritSec.Enter();
    for (MicListener* existing : mListeners) {
        if (existing == listener) {
            break;
        }
    }
    mListeners.push_back(listener);
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA2570.
bool MicHwManager::RemoveListener(MicListener* listener) {
    mCritSec.Enter();
    bool removed = false;
    for (MicListener** entry = mListeners.begin(); entry != mListeners.end(); ++entry) {
        if (*entry == listener) {
            mListeners.erase(entry);
            removed = true;
            break;
        }
    }
    mCritSec.Exit();
    return removed;
}

// Reconstructed from eboot.elf at 0xA2610.
void MicHwManager::_CheckConnectsAndDisconnects() {
    mCritSec.Enter();
    for (MicHwPlatform* platform : mPlatforms) {
        platform->CheckConnectsAndDisconnects();
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA2680.
void MicHwManager::MicThreadPoll() {
    mCritSec.Enter();
    for (Mic* mic : mMics) {
        mic->MicThreadPoll();
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA26F0.
Mic* MicHwManager::GetFreeMic(int type) {
    mCritSec.Enter();
    Mic* result = nullptr;
    for (Mic* mic : mMics) {
        if (mic->GetType() == type && mic->GetStatus() == 0) {
            result = mic;
            break;
        }
    }
    mCritSec.Exit();
    return result;
}

// Reconstructed from eboot.elf at 0xA2790.
void MicHwManager::MarkMicsChanged() {
    mCritSec.Enter();
    mMicsChanged = true;
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA2800 (0xA27C0 deletes). The vectors
// free their storage without deleting the mics.
MicHwManager::~MicHwManager() {}

// Reconstructed from eboot.elf at 0xA27E0.
void MicHwManager::_PlatformInit() {}

// Reconstructed from eboot.elf at 0xA27F0.
void MicHwManager::_PlatformTerminate() {}
