#pragma once

#include <cstddef>

#include "persistence/core/PersistentData.h"
#include "persistence/storage/StorageRef_PS4.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"

class DataArray;

// A user's saved game data (persistence/Profile.o). The game derives its
// profile from it; ProfileStorageClient saves it to its pad's StorageRef.
// It keeps its own dirty flag: dirtying can be switched off while a save
// is loaded, and each MakeDirty counts. The out-of-line members are in the
// second persistence block (0x11B1AF0-0x11B1FD0); the inline IsDirty and
// GetSaveVersion are folded into game copies. The vtable at 0x19A59B0 has
// 14 slots, with MsgSink's at 0x19A5A30 (offset 24); the object is 200
// bytes.
class Profile : public PersistentData, public MsgSink {
public:
    explicit Profile(int padNum);  // 0x11B1AF0
    // Slots 0-1: 0x11B1B50, 0x11B1C70; MsgSink thunks 0x11B1BE0,
    // 0x11B1CF0.
    ~Profile() override;

    // Slot 2: counts the change while dirtying is enabled.
    void MakeDirty() override;  // 0x11B1D90
    // Slot 3.
    void MakeClean() override;  // 0x11B1DB0
    // Slot 4. Inline in the map's build; the binary's copy is folded into
    // the game code at 0xD59460.
    bool IsDirty() override {
        return mProfileDirty;
    }
    // Slot 7 (MsgSink thunk 0x11B1ED0): handles "delete_all".
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x11B1DE0
    // Slot 8. Inline in the map's build; emitted at 0x11B1FB0.
    virtual void Poll() {}
    // Slot 9. Inline in the map's build; emitted at 0x11B1FC0.
    virtual bool CanSave() const {
        return false;
    }
    // Slot 10: marks the profile dirty; the game's profile clears its data.
    virtual void DeleteAll();  // 0x11B1D80
    // Slot 11.
    virtual void EnableDirtying(bool enable);  // 0x11B1DD0
    // Slot 12. Inline in the map's build; emitted at 0x11B1FD0.
    virtual const char* GetSaveFileName() const {
        return "game.save";
    }
    // Slot 13: the revision ProfileStorageClient writes the save at. Inline
    // in the map's build; the binary's copy is folded into the game code at
    // 0xD59480.
    virtual int GetSaveVersion() const {
        return 0;
    }

    // Field names are not in the reference map.
    bool mProfileDirty;
    bool mDirtyingEnabled;
    // The changes since the last MakeClean, from -1. The evidence for the
    // name is weak: nothing in the module reads it.
    int mDirtyCount;
    // Whether ExecuteSave may save: set when no save was found for the pad,
    // so that a new profile can be written. The evidence for the name is
    // weak; the game code sets it after a load.
    bool mReadyToSave;
    StorageRef_PS4 mStorage;
    // The pad the profile belongs to, or -1.
    int mPadNum;
};

static_assert(offsetof(Profile, mProfileDirty) == 32);
static_assert(offsetof(Profile, mDirtyingEnabled) == 33);
static_assert(offsetof(Profile, mDirtyCount) == 36);
static_assert(offsetof(Profile, mReadyToSave) == 40);
static_assert(offsetof(Profile, mStorage) == 48);
static_assert(offsetof(Profile, mPadNum) == 192);
static_assert(sizeof(Profile) == 200);
