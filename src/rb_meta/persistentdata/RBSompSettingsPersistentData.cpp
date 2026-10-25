// RBSompSettingsPersistentData.o, newer than the map (0xD65CB0 to
// 0xD663EF; RBSystemOptions.o follows at 0xD663F0). The static initializer
// at 0xD663B0 sets the shared header globals every rb_meta object
// initializes, among them this object's copy of the default SOMP settings
// (0x1B1FD30); only that copy is modelled, as sDefaultSompSessionSettings.
#include "rb_meta/persistentdata/RBSompSettingsPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

namespace {

// This object's copy of the shared header's defaults (0x1B1FD30). Name not
// in the reference map.
const SompSessionSettings sDefaultSompSessionSettings = GetDefaultSompSessionSettings();

}  // namespace

// Reconstructed from eboot.elf at 0xD65CB0. Clear is inlined.
RBSompSettingsPersistentData::RBSompSettingsPersistentData(RBProfile* profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD65D00.
void RBSompSettingsPersistentData::Clear() {
    mSettings = sDefaultSompSessionSettings;
}

// Reconstructed from eboot.elf at 0xD65D20.
void RBSompSettingsPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 1, 0);
        revisionStream.BeginWrite();
        for (int choice : mSettings.mFirstChoices) {
            revisionStream.WriteEndian(&choice, sizeof(choice));
        }
        for (int choice : mSettings.mLastChoices) {
            revisionStream.WriteEndian(&choice, sizeof(choice));
        }
        for (bool flag : mSettings.mFlags) {
            revisionStream.Write(&flag, sizeof(flag));
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD65F40. The choices are read into
// copies of the defaults and stored after the flags; every value needs
// revision 1, so older blocks keep the defaults' choices and the current
// flags.
void RBSompSettingsPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 1, 0);
        if (revisionStream.BeginRead()) {
            int firstChoices[4];
            int lastChoices[4];
            for (int i = 0; i < 4; ++i) {
                firstChoices[i] = sDefaultSompSessionSettings.mFirstChoices[i];
                lastChoices[i] = sDefaultSompSessionSettings.mLastChoices[i];
            }
            for (int& choice : firstChoices) {
                revisionStream >> Revision::AsOfOp<int>(&choice, 1);
            }
            for (int& choice : lastChoices) {
                revisionStream >> Revision::AsOfOp<int>(&choice, 1);
            }
            for (bool& flag : mSettings.mFlags) {
                revisionStream >> Revision::AsOfOp<bool>(&flag, 1);
            }
            for (int i = 0; i < 4; ++i) {
                mSettings.mFirstChoices[i] = firstChoices[i];
                mSettings.mLastChoices[i] = lastChoices[i];
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD66390. The deleting destructor is at
// 0xD663A0.
RBSompSettingsPersistentData::~RBSompSettingsPersistentData() {}
