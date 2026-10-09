#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"

class RndBufferCollection;

// The buffer collections a window draws into. Name not in the reference map.
struct RndBufferCollectionList {
    RndBufferCollection** mCollections;
    unsigned long mCount;
};

// Render window: a presentation surface with one or more buffer collections.
// The vtable is at 0x1901F38; the constructor and the non-virtual helpers are
// in the RndWindow object at 0x4486F0-0x448840.
class RndWindow {
public:
    RndWindow();           // 0x4486F0
    virtual ~RndWindow();  // 0x448710, 0x448720

    // Slots 2, 3, and 6 are not in the reference map.
    virtual unsigned long _GetActiveBufferIndex() const = 0;           // slot 2
    virtual RndBufferCollectionList GetBufferCollections() const = 0;  // slot 3
    virtual void Poll();            // slot 4 at 0x448820
    // Called as each frame starts drawing the window.
    virtual void CheckForResize();  // slot 5 at 0x448830
    virtual bool _UnknownSlot6();   // slot 6 at 0x448840

    // The first collection's size, or zero without one. Names not in the
    // reference map.
    Vector2i GetSize() const;                    // 0x448730
    unsigned int GetShadingMode() const;                  // 0x448760
    void SetShadingMode(unsigned int mode);               // 0x448780
    unsigned int GetBufferInspectionMode() const;         // 0x4487C0
    void SetBufferInspectionMode(unsigned int mode);      // 0x4487E0

    int mUnknown8;  // Name not in the reference map.
};

static_assert(sizeof(RndWindow) == 16);

// Window that presents a single buffer collection, which it may own. Its
// members are emitted with the inline functions at 0x11B2CD0-0x11B2E00. Name
// not in the reference map.
class RndBufferedWindow : public RndWindow {
public:
    RndBufferedWindow(unsigned int bufferFlags, bool createBuffers);  // 0x11B2CD0
    ~RndBufferedWindow() override;  // 0x11B2D40, 0x11B2D90

    unsigned long _GetActiveBufferIndex() const override;           // 0x11B2DE0
    RndBufferCollectionList GetBufferCollections() const override;  // 0x11B2DF0
    void SetBufferCollection(RndBufferCollection* buffers);       // 0x11B2E00

    // Field names are not in the reference map.
    bool mOwnsBuffers;
    RndBufferCollection* mBuffers;
    RndBufferCollection* mActiveBuffers;
};

static_assert(offsetof(RndBufferedWindow, mOwnsBuffers) == 12);
static_assert(offsetof(RndBufferedWindow, mBuffers) == 16);
static_assert(offsetof(RndBufferedWindow, mActiveBuffers) == 24);
static_assert(sizeof(RndBufferedWindow) == 32);
