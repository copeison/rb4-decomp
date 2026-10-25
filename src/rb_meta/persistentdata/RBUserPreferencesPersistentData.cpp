// RBUserPreferencesPersistentData.o, newer than the map (0xD68800 to
// 0xD6907F). The object also emits Revision::AsOfOp<unsigned long> (vtable
// 0x197E258: destructors 0xD68C40 and 0xD68FD0, Read 0xD68FE0). The static
// initializer at 0xD69040 sets the shared header globals every rb_meta
// object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBUserPreferencesPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reads a 64-bit value saved as 32 bits, as the AsOfOp's Read (0xD68FE0)
// inlines it. Name not in the reference map.
static BinStream& operator>>(BinStream& stream, unsigned long& value) {
    unsigned int saved;
    stream.ReadEndian(&saved, sizeof(saved));
    value = saved;
    return stream;
}

// Reconstructed from eboot.elf at 0xD68800. Clear is inlined.
RBUserPreferencesPersistentData::RBUserPreferencesPersistentData(RBProfile* profile)
    : mProfile(profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD688A0.
void RBUserPreferencesPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSymbolValue = Symbol();
    mLongValue = static_cast<unsigned long>(-1);
    mFlags[0] = true;
    mFlags[2] = false;
    mFlags[3] = false;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD68920.
void RBUserPreferencesPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 1, 0);
        revisionStream.BeginWrite();
        revisionStream << mSymbolValue;
        int value = static_cast<int>(mLongValue);
        revisionStream.WriteEndian(&value, sizeof(value));
        for (bool flag : mFlags) {
            revisionStream.Write(&flag, sizeof(flag));
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD68A50.
void RBUserPreferencesPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 1, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<Symbol>(&mSymbolValue, 0);
            revisionStream >> Revision::AsOfOp<unsigned long>(&mLongValue, 0);
            revisionStream >> Revision::AsOfOp<bool>(&mFlags[0], 0);
            revisionStream >> Revision::AsOfOp<bool>(&mFlags[1], 0);
            revisionStream >> Revision::AsOfOp<bool>(&mFlags[2], 1);
            revisionStream >> Revision::AsOfOp<bool>(&mFlags[3], 1);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD68C50.
bool RBUserPreferencesPersistentData::IsFirstFlagSet() const {
    return mFlags[0];
}

// Reconstructed from eboot.elf at 0xD68C60.
void RBUserPreferencesPersistentData::SetFirstFlag(bool set) {
    if (mFlags[0] != set) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        mFlags[0] = set;
        MakeDirty();
        MemPopHeap();
    }
}

// Reconstructed from eboot.elf at 0xD68CE0.
bool RBUserPreferencesPersistentData::IsSecondFlagSet() const {
    return mFlags[1];
}

// Reconstructed from eboot.elf at 0xD68CF0.
void RBUserPreferencesPersistentData::SetSecondFlag(bool set) {
    if (mFlags[1] != set) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        mFlags[1] = set;
        MakeDirty();
        MemPopHeap();
    }
}

// Reconstructed from eboot.elf at 0xD68D70.
Symbol RBUserPreferencesPersistentData::GetSymbolValue() const {
    return mSymbolValue;
}

// Reconstructed from eboot.elf at 0xD68D80.
unsigned long RBUserPreferencesPersistentData::GetLongValue() const {
    return mLongValue;
}

// Reconstructed from eboot.elf at 0xD68D90.
void RBUserPreferencesPersistentData::SetSymbolValue(Symbol value) {
    if (mSymbolValue != value) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        mSymbolValue = value;
        MakeDirty();
        MemPopHeap();
    }
}

// Reconstructed from eboot.elf at 0xD68E10.
void RBUserPreferencesPersistentData::SetLongValue(unsigned long value) {
    if (mLongValue != value) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        mLongValue = value;
        MakeDirty();
        MemPopHeap();
    }
}

// Reconstructed from eboot.elf at 0xD68E90.
bool RBUserPreferencesPersistentData::IsThirdFlagSet() const {
    return mFlags[2];
}

// Reconstructed from eboot.elf at 0xD68EA0.
bool RBUserPreferencesPersistentData::IsFourthFlagSet() const {
    return mFlags[3];
}

// Reconstructed from eboot.elf at 0xD68EB0.
void RBUserPreferencesPersistentData::SetThirdFlag(bool set) {
    if (mFlags[2] != set) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        mFlags[2] = set;
        MakeDirty();
        MemPopHeap();
    }
}

// Reconstructed from eboot.elf at 0xD68F30.
void RBUserPreferencesPersistentData::SetFourthFlag(bool set) {
    if (mFlags[3] != set) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        mFlags[3] = set;
        MakeDirty();
        MemPopHeap();
    }
}

// Reconstructed from eboot.elf at 0xD68FB0. The deleting destructor is at
// 0xD68FC0.
RBUserPreferencesPersistentData::~RBUserPreferencesPersistentData() {}
