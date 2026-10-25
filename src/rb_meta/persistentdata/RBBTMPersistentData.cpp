// RBBTMPersistentData.o, newer than the map (0xD46010 to 0xD4649B). The
// static initializer at 0xD46460 sets the shared header globals every
// rb_meta object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBBTMPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reconstructed from eboot.elf at 0xD46010. The record's constructor is
// inlined; Clear is a tail call.
RBBTMPersistentData::RBBTMPersistentData(RBProfile* profile) : mProfile(profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD46080. The record is replaced by one
// whose parent is this data, but the assignment keeps the old parent.
void RBBTMPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mPlaythroughsStarted = 0;
    mPlaythroughsEnded = 0;
    mBestPlaythrough = RBBTMPlaythroughRecord(this);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD46150. The deleting destructor is at
// 0xD46160.
RBBTMPersistentData::~RBBTMPersistentData() {}

// Reconstructed from eboot.elf at 0xD46170.
const char* RBBTMPersistentData::_GetProfileName() const {
    if (mProfile == nullptr) {
        return "NULL_PROFILE_POINTER!!!";
    }
    return mProfile->GetPlatformName();
}

// Reconstructed from eboot.elf at 0xD46190.
void RBBTMPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 0, 0);
        revisionStream.BeginWrite();
        int value = mPlaythroughsStarted;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mPlaythroughsEnded;
        revisionStream.WriteEndian(&value, sizeof(value));
        mBestPlaythrough.Save(revisionStream);
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD46280. A block that cannot be read
// only fetches the profile's name for a stripped log.
void RBBTMPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 0, 0);
        if (revisionStream.BeginRead()) {
            revisionStream.ReadEndian(&mPlaythroughsStarted, sizeof(mPlaythroughsStarted));
            revisionStream.ReadEndian(&mPlaythroughsEnded, sizeof(mPlaythroughsEnded));
            mBestPlaythrough.Load(revisionStream);
        } else {
            static_cast<void>(_GetProfileName());
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD46380.
void RBBTMPersistentData::StartPlaythrough() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    ++mPlaythroughsStarted;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD463E0.
void RBBTMPersistentData::EndPlaythrough(const RBBTMPlaythroughRecord& record) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    ++mPlaythroughsEnded;
    if (record.mScore > mBestPlaythrough.mScore) {
        mBestPlaythrough = record;
    }
    MemPopHeap();
}
