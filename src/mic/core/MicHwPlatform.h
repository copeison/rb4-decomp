#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

// Base of the platform microphone managers such as MicHwManager_FMOD. They
// register with gMicHwManager, which drives them through the non-virtual
// wrappers at 0xA2D70 to 0xA2DE0. The class is not in the reference map; its
// slots follow MicHwManager_FMOD's vtable at 0x18F0CF0. Names not in the
// reference map unless noted.
class MicHwPlatform {
public:
    // EASTL vector of bus paths passed to SetBusPaths.
    struct PathList {
        Symbol* mBegin;
        Symbol* mEnd;
    };

    MicHwPlatform() : mInitialized(false) {}
    virtual ~MicHwPlatform() {}                        // slots 0-1
    virtual bool _Init() = 0;                          // slot 2. Inferred from the map.
    virtual void SetBusPaths(const PathList& paths) = 0;  // slot 3
    virtual void _SetupMicArray(int numMics) = 0;      // slot 4
    virtual void _Poll() = 0;                          // slot 5. Inferred from the map.
    // Slot 6: releases what the platform holds when the mic list shuts down.
    virtual void ClearBusPaths() = 0;
    virtual void _CheckConnectsAndDisconnects() = 0;   // slot 7

    // Creates the platform's mics once. At 0xA2D70.
    void Init(int numMics);
    void Poll();       // 0xA2D90
    // Shuts an initialized platform down. At 0xA2DA0.
    void Terminate();
    void CheckConnectsAndDisconnects();  // 0xA2DD0
    // Forwards to gMicHwManager. At 0xA2DE0.
    void MarkMicsChanged();

    bool mInitialized;
};

static_assert(sizeof(MicHwPlatform) == 16);
