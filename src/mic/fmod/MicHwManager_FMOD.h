#pragma once

#include <cstddef>

#include "audio/fmod/api/fmod_api.h"
#include "utl/text/Symbol.h"

class Mic_FMOD;

// FMOD microphone platform. It creates the FMOD mics in the engine's mic list,
// attaches them to the connected "GENERAL" record drivers, and owns the
// Studio buses that carry the mic signal. The single instance at 0x19F2EE0
// registers with gMicHwManager. The vtable is at 0x18F0CF0; the object is 56
// bytes.
class MicHwManager_FMOD {
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

    // EASTL vector of bus paths passed to SetBusPaths. Name not in the
    // reference map.
    struct PathList {
        Symbol* mBegin;
        Symbol* mEnd;
    };

    MicHwManager_FMOD();                  // Inlined into the static initializer at 0x275DB0.
    virtual ~MicHwManager_FMOD();         // slots 0-1: 0x275490, 0x275BC0
    virtual bool _Init();                 // slot 2: 0x275C00. Inferred from the map.
    // Slot 3 at 0x2754C0. Name not in the reference map.
    virtual void SetBusPaths(const PathList& paths);
    virtual void _SetupMicArray(int numMics);  // slot 4: 0x275830
    // Slot 5 at 0x2758A0: retries a failed bus binding. Inferred from the map.
    virtual void _Poll();
    // Slot 6 at 0x2758B0. Name not in the reference map.
    virtual void ClearBusPaths();
    virtual void _CheckConnectsAndDisconnects();  // slot 7: 0x275950

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
    bool mUnknown8;
    BusRouteList mRoutes;
    bool mBindFailed;
};

static_assert(sizeof(MicHwManager_FMOD::BusRoute) == 24);
static_assert(offsetof(MicHwManager_FMOD, mRoutes) == 16);
static_assert(offsetof(MicHwManager_FMOD, mBindFailed) == 48);
static_assert(sizeof(MicHwManager_FMOD) == 56);

extern MicHwManager_FMOD gMicHwManagerFMOD;  // Name not in the reference map.
