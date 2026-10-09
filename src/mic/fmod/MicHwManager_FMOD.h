#pragma once

#include <cstddef>

#include "audio/fmod/api/fmod_api.h"
#include "mic/core/MicHwPlatform.h"
#include "utl/text/Symbol.h"

class Mic_FMOD;

// FMOD microphone platform. It creates the FMOD mics in the engine's mic list,
// attaches them to the connected "GENERAL" record drivers, and owns the
// Studio buses that carry the mic signal. The single instance at 0x19F2EE0
// registers with gMicHwManager. The vtable is at 0x18F0CF0; the object is 56
// bytes.
class MicHwManager_FMOD : public MicHwPlatform {
public:
    // A Studio bus that carries mic audio, with its authored volume. Names
    // not in the reference map.
    struct BusRoute {
        Symbol mPath;
        FMOD::Studio::Bus* mBus;
        float mAuthoredVolume;
    };

    // EASTL vector of routes. Name not in the reference map.
    struct BusRouteList {
        BusRoute* mBegin;
        BusRoute* mEnd;
        BusRoute* mCapacity;
        void* mAllocator;
    };

    MicHwManager_FMOD();                  // Inlined into the static initializer at 0x275DB0.
    ~MicHwManager_FMOD() override;        // slots 0-1: 0x275490, 0x275BC0
    bool _Init() override;                // slot 2: 0x275C00
    void SetBusPaths(const PathList& paths) override;  // slot 3: 0x2754C0
    // Slot 4 at 0x275830: starts the mic reader thread and adds the mics.
    void _SetupMicArray(int numMics) override;
    // Slot 5 at 0x2758A0: retries a failed bus binding.
    void _Poll() override;
    void ClearBusPaths() override;        // slot 6: 0x2758B0
    void _CheckConnectsAndDisconnects() override;  // slot 7: 0x275950

    // Binds every route to its Studio bus; any failure unbinds the routes
    // and flags a retry. At 0x275630. Name not in the reference map.
    void _BindBuses();
    // Accepts only drivers whose name ends in "GENERAL". At 0x275910.
    bool _IsValidMicName(const char* name);
    // Per-route controls at 0x275740, 0x275780 and 0x2757C0. Names not in the
    // reference map.
    void SetBusVolume(int index, float volume);
    void SetBusMute(int index, bool mute);
    FMOD::ChannelGroup* GetBusChannelGroup(int index);
    // Releases every route's bus check and empties the list. Shared by
    // SetBusPaths and ClearBusPaths. Name not in the reference map.
    void _ReleaseRoutes();

    // Field names are not in the reference map.
    BusRouteList mRoutes;
    bool mBindFailed;
};

static_assert(sizeof(MicHwManager_FMOD::BusRoute) == 24);
static_assert(offsetof(MicHwManager_FMOD, mRoutes) == 16);
static_assert(offsetof(MicHwManager_FMOD, mBindFailed) == 48);
static_assert(sizeof(MicHwManager_FMOD) == 56);

extern MicHwManager_FMOD gMicHwManagerFMOD;  // Name not in the reference map.
