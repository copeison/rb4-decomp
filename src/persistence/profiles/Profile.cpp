#include "persistence/profiles/Profile.h"

#include "utl/data/DataArray.h"
#include "utl/text/Symbol.h"

// Reconstructed from eboot.elf at 0x11B1AF0.
Profile::Profile(int padNum)
    : mProfileDirty(true), mDirtyingEnabled(true), mDirtyCount(-1), mReadyToSave(false), mPadNum(padNum) {}

// Reconstructed from eboot.elf at 0x11B1B50.
Profile::~Profile() {
    EnableDirtying(false);
}

// Reconstructed from eboot.elf at 0x11B1D80.
void Profile::DeleteAll() {
    MakeDirty();
}

// Reconstructed from eboot.elf at 0x11B1D90.
void Profile::MakeDirty() {
    if (mDirtyingEnabled) {
        if (!mProfileDirty) {
            mProfileDirty = true;
        }
        ++mDirtyCount;
    }
}

// Reconstructed from eboot.elf at 0x11B1DB0.
void Profile::MakeClean() {
    if (mProfileDirty) {
        mProfileDirty = false;
    }
    if (mDirtyCount != -1) {
        mDirtyCount = -1;
    }
}

// Reconstructed from eboot.elf at 0x11B1DD0.
void Profile::EnableDirtying(bool enable) {
    mDirtyingEnabled = enable;
}

// Reconstructed from eboot.elf at 0x11B1DE0.
DataNode Profile::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol deleteAll;
    if (deleteAll == Symbol()) {
        deleteAll = Symbol("delete_all");
    }
    if (type == deleteAll) {
        DeleteAll();
        return DataNode(0);
    }
    return DataNode();
}
