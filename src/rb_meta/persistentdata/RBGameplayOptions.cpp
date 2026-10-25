// rb_meta/RBGameplayOptions.o (0xD52AF0 to 0xD5342F). The object also holds
// Revision::AtOp<bool> (vtable 0x197DA80: 0xD52FF0, 0xD53380, Read
// 0xD53390), inline in utl/streams/RevisionStream.h, and PersistentData's
// vtable (0x197DA38: 0xD53360, 0xD53370), which the constructor installs
// before GameplayOptions' constructor runs. The static initializer at
// 0xD533F0 sets the shared header globals every rb_meta object initializes;
// it is not modelled.
#include "rb_meta/persistentdata/RBGameplayOptions.h"

#include "audio/fmod/system/FmodPlatform.h"
#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

static_assert(offsetof(GameplayOptions, mOptions) == 8);
static_assert(sizeof(GameplayOptions::mOptions) >= 0x30);

// Reconstructed from eboot.elf at 0xD52AF0.
RBGameplayOptions::RBGameplayOptions(RBProfile* profile) {
    mParent = profile;
}

// Reconstructed from eboot.elf at 0xD53300. The deleting destructor is at
// 0xD53310.
RBGameplayOptions::~RBGameplayOptions() {}

// Reconstructed from eboot.elf at 0xD52B40.
void RBGameplayOptions::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        const SavedValues& values = _Values();
        RevisionStream revisionStream(stream, 11, 0);
        revisionStream.BeginWrite();
        bool flag = values.mGuitarLefty;
        revisionStream.Write(&flag, sizeof(flag));
        flag = values.mDrumLefty;
        revisionStream.Write(&flag, sizeof(flag));
        int value = values.mGuitarSoloMode;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = values.mDrumFillLogic;
        revisionStream.WriteEndian(&value, sizeof(value));
        flag = values.mVocalImprovEnabled;
        revisionStream.Write(&flag, sizeof(flag));
        unsigned int index = values.mBreakneckSpeedModifierIndex;
        revisionStream.WriteEndian(&index, sizeof(index));
        index = values.mMicVolumeIndices[0];
        revisionStream.WriteEndian(&index, sizeof(index));
        index = values.mVocalStemVolumeIndex;
        revisionStream.WriteEndian(&index, sizeof(index));
        index = values.mIGSVolumeIndex;
        revisionStream.WriteEndian(&index, sizeof(index));
        flag = values.mGameplayFlagEnabled;
        revisionStream.Write(&flag, sizeof(flag));
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD52CE0.
void RBGameplayOptions::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        SavedValues& values = _Values();
        RevisionStream revisionStream(stream, 11, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<bool>(&values.mGuitarLefty, 0);
            revisionStream >> Revision::AsOfOp<bool>(&values.mDrumLefty, 0);
            int guitarSoloMode = 1;
            revisionStream >> Revision::AsOfOp<int>(&guitarSoloMode, 3);
            values.mGuitarSoloMode = guitarSoloMode;
            int drumFillLogic = 2;
            revisionStream >> Revision::AsOfOp<int>(&drumFillLogic, 3);
            values.mDrumFillLogic = drumFillLogic;
            revisionStream >> Revision::AsOfOp<bool>(&values.mVocalImprovEnabled, 4);
            bool droppedFlag;
            revisionStream >> Revision::AtOp<bool>(&droppedFlag, 9);
            revisionStream >> Revision::AsOfOp<unsigned int>(&values.mBreakneckSpeedModifierIndex, 5);
            revisionStream >> Revision::AsOfOp<unsigned int>(&values.mMicVolumeIndices[0], 6);
            values.mMicVolumeIndices[1] = values.mMicVolumeIndices[0];
            values.mMicVolumeIndices[2] = values.mMicVolumeIndices[0];
            revisionStream >> Revision::AsOfOp<unsigned int>(&values.mVocalStemVolumeIndex, 7);
            revisionStream >> Revision::AsOfOp<unsigned int>(&values.mIGSVolumeIndex, 8);
            revisionStream >> Revision::AsOfOp<bool>(&values.mGameplayFlagEnabled, 11);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD53000.
void RBGameplayOptions::_OnOptionsChanged() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD530E0.
void RBGameplayOptions::_OnIGSVolumeChanged() {
    gFmodBusInterface->SetVcaVolumeDb("vca:/IGS", 2, GetIGSVolumeDecibels(), 0.0f);
}

// Reconstructed from eboot.elf at 0xD532F0.
void RBGameplayOptions::SetOptions(const GameplayOptions& options) {
    GameplayOptions::operator=(options);
}
