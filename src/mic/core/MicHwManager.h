#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"
#include "utl/containers/Vector.h"

class Mic;
class MicHwPlatform;

// Receives a call whenever the mic list changes. Registered through
// MicHwManager::AddListener; no recovered code registers one. Name not in
// the reference map.
class MicListener {
public:
    virtual ~MicListener();                // slots 0-1
    virtual void OnMicsChanged() = 0;      // slot 2
};

// The engine's microphone list (mic/MicHwManager.o), the single instance at
// 0x19C8FC0. Platform managers create its mics; the mic reader thread polls
// them. The vtable is at 0x18E5070; the object is 128 bytes. Names not in
// the reference map unless noted.
class MicHwManager {
public:
    // GetNextAvailableMicID accepts any type for this value.
    static constexpr int kAnyMicType = 3;

    MicHwManager() : mInitialized(false), mMicsChanged(false) {}
    virtual ~MicHwManager();  // slots 0-1: 0xA1C20 (jumps to 0xA2800), 0xA27C0
    // Slots 2-3 at 0xA27E0 and 0xA27F0 are empty and have no caller. The
    // names pair them with Init and Terminate as platform hooks; a guess.
    // Names not in the reference map.
    virtual void _PlatformInit();
    virtual void _PlatformTerminate();

    void Init();  // 0xA1C30. In the map.
    // Starts the mic reader thread unless it runs. At 0xA1C40.
    void _StartMicReaderThread();
    static MicHwManager* GetInstance();  // 0xA1C60
    // Notifies the listeners after a change, polls the platforms and then
    // every mic. At 0xA1C70. In the map.
    void Poll();
    void _NotifyMicsChanged();  // 0xA1D80
    // Stops the reader thread, shuts the platforms down and deletes the
    // mics. At 0xA1DF0.
    void Terminate();
    void RegisterPlatform(MicHwPlatform* platform);  // 0xA1EB0
    void AddMic(Mic* mic);                            // 0xA1F70
    int GetNumConnectedMics();                        // 0xA2060
    // The map has these on MicHwManager_FMOD.
    bool IsMicConnected(int index);       // 0xA20E0
    Mic* GetMic(int index);               // 0xA2160
    int GetNextAvailableMicID(int type);  // 0xA21D0
    // A forced capture or release skips the index check. The checks accept
    // an index equal to the mic count.
    bool CaptureMic(int index, bool force);  // 0xA22E0
    bool ReleaseMic(int index, bool force);  // 0xA2370
    void ReleaseAllMics();                   // 0xA2400
    void AddListener(MicListener* listener);     // 0xA2460
    bool RemoveListener(MicListener* listener);  // 0xA2570
    // Run by the mic reader thread: the platforms' connection checks every
    // 100 passes and every mic's MicThreadPoll on each pass. At 0xA2610 and
    // 0xA2680.
    void _CheckConnectsAndDisconnects();
    void MicThreadPoll();
    // Returns an unbound mic of the given type. At 0xA26F0.
    Mic* GetFreeMic(int type);
    // Flags the mic list as changed. At 0xA2790.
    void MarkMicsChanged();

    bool mInitialized;
    bool mMicsChanged;
    CritSec mCritSec;
    eastl::vector<Mic*> mMics;
    eastl::vector<MicListener*> mListeners;
    eastl::vector<MicHwPlatform*> mPlatforms;
};

static_assert(offsetof(MicHwManager, mInitialized) == 8);
static_assert(offsetof(MicHwManager, mCritSec) == 16);
static_assert(offsetof(MicHwManager, mMics) == 32);
static_assert(offsetof(MicHwManager, mListeners) == 64);
static_assert(offsetof(MicHwManager, mPlatforms) == 96);
static_assert(sizeof(MicHwManager) == 128);

extern MicHwManager gMicHwManager;  // 0x19C8FC0
