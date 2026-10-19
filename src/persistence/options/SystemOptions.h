#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "persistence/core/PersistentData.h"
#include "persistence/storage/StorageRef_PS4.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"

class BinStream;
class DataArray;

// The console-wide options (persistence/SystemOptions.o): screen and audio
// calibration, saved by SystemOptionsStorageClient to their own
// StorageRef. Changing one exports "system_options_dirty". The out-of-line
// members are in the second persistence block (0x11B1FE0-0x11B2C73). The
// vtable at 0x19A5A60 has nine slots, with MsgSource's at 0x19A5AB8 (offset
// 24); the object is 216 bytes, from the small-block pool.
class SystemOptions : public PersistentData, public MsgSource {
public:
    POOL_OVERLOAD(SystemOptions)

    // The revision Save writes. Name not in the reference map.
    static constexpr int kRevision = 5;

    SystemOptions();  // 0x11B1FE0
    // Slots 0-1: 0x11B2A20, 0x11B2AB0; MsgSource thunks 0x11B2B50,
    // 0x11B2BE0.
    ~SystemOptions() override;

    // Slot 2: exports "system_options_dirty" and marks the options dirty.
    void MakeDirty() override;  // 0x11B2770
    // Slot 5.
    void Save(BinStream& stream) const override;  // 0x11B2220
    // Slot 6: reads the values the stored revision has, upgrades the
    // offsets of older or uncalibrated saves, and marks the options loaded
    // even when the block cannot be read.
    void Load(BinStream& stream) override;  // 0x11B2310
    // Slot 7 (MsgSource thunk 0x11B2940): handles
    // "on_system_options_loaded".
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x11B2850
    // Slot 8: restores the defaults.
    virtual void Reset();  // 0x11B2050

    // Each setter stores the value and marks the options dirty when it
    // changes by more than 0.0001, or always when forced. The offsets are
    // clamped to 300 ms either way.
    // Not emitted in this build; Reset and Load inline it.
    void SetOverscan(float overscan, bool force) {
        if (force || __builtin_fabsf(mOverscan - overscan) > 0.0001f) {
            mOverscan = overscan;
            MakeDirty();
        }
    }
    void SetAudioOffset(float offset, bool force);   // 0x11B2100
    void SetVideoOffset(float offset, bool force);   // 0x11B2150
    void SetDialogVolume(float volume, bool force);  // 0x11B21A0
    void SetGammaValue(float gamma, bool force);     // 0x11B21D0
    void SetHasCalibrated(bool hasCalibrated);       // 0x11B2200
    // Inlined into Handle.
    void _OnSystemOptionsLoaded() {
        mLoaded = true;
    }

    // Field names are not in the reference map; the setters name them.
    float mOverscan;
    float mAudioOffset;
    float mVideoOffset;
    float mDialogVolume;
    float mGammaValue;
    // Set once loading finished, whether or not a save was read.
    bool mLoaded;
    bool mHasCalibrated;
    StorageRef_PS4 mStorage;
};

static_assert(offsetof(SystemOptions, mOverscan) == 48);
static_assert(offsetof(SystemOptions, mGammaValue) == 64);
static_assert(offsetof(SystemOptions, mLoaded) == 68);
static_assert(offsetof(SystemOptions, mHasCalibrated) == 69);
static_assert(offsetof(SystemOptions, mStorage) == 72);
static_assert(sizeof(SystemOptions) == 216);
