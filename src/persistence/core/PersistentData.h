#pragma once

#include <cstddef>

class BinStream;

// Data that tracks whether it changed since it was last saved. Every member
// is inline; the game code keeps the binary's copies of MakeClean (0x9D4120)
// and IsDirty (0x9D4130), which the persistence vtables share.
class IDirty {
public:
    IDirty() : mDirty(false) {}
    virtual ~IDirty() {}

    // Slot 2.
    virtual void MakeDirty() = 0;
    // Slot 3.
    virtual void MakeClean() {
        mDirty = false;
    }
    // Slot 4.
    virtual bool IsDirty() {
        return mDirty;
    }

    bool mDirty;  // Name not in the reference map.
};

static_assert(offsetof(IDirty, mDirty) == 8);
static_assert(sizeof(IDirty) == 16);

// Dirty-tracked data that a profile or the system options save. Every member
// is inline. Saving a child marks its parent dirty through MakeDirty, whose
// binary copy is at 0x9D4110.
class PersistentData : public IDirty {
public:
    PersistentData() : mParent(nullptr) {}
    ~PersistentData() override {}

    // Slot 2: marks the parent dirty.
    void MakeDirty() override {
        if (mParent != nullptr) {
            mParent->MakeDirty();
        }
    }
    // Slot 5.
    virtual void Save(BinStream& stream) const = 0;
    // Slot 6.
    virtual void Load(BinStream& stream) = 0;

    // The data this one is saved within, or null. Name not in the reference
    // map; it rests on MakeDirty's forwarding.
    IDirty* mParent;
};

static_assert(offsetof(PersistentData, mParent) == 16);
static_assert(sizeof(PersistentData) == 24);
