// rb_meta/FTUEPersistentData.o (0xD45270 to 0xD456EB). The static
// initializer at 0xD456B0 sets the shared header globals every rb_meta
// object initializes; it is not modelled.
#include "rb_meta/persistentdata/FTUEPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reconstructed from eboot.elf at 0xD45270.
FTUEPersistentData::FTUEPersistentData(RBProfile* profile) {
    mParent = profile;
}

// Reconstructed from eboot.elf at 0xD452B0. The deleting destructor is at
// 0xD452E0.
FTUEPersistentData::~FTUEPersistentData() {}

// Reconstructed from eboot.elf at 0xD45320.
void FTUEPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 0, 0);
        revisionStream.BeginWrite();
        unsigned int count = static_cast<unsigned int>(mSeenEvents.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (const Symbol* event = mSeenEvents.begin(); event != mSeenEvents.end(); ++event) {
            revisionStream << *event;
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD45420.
void FTUEPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 0, 0);
        if (revisionStream.BeginRead()) {
            unsigned int count;
            revisionStream.ReadEndian(&count, sizeof(count));
            mSeenEvents.resize(count);
            for (Symbol* event = mSeenEvents.begin(); event != mSeenEvents.end(); ++event) {
                revisionStream >> *event;
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD45540.
void FTUEPersistentData::MarkEventSeen(Symbol event) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (!HasSeenEvent(event)) {
        mSeenEvents.push_back(event);
    }
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD45670.
bool FTUEPersistentData::HasSeenEvent(Symbol event) const {
    for (const Symbol* seen = mSeenEvents.begin(); seen != mSeenEvents.end(); ++seen) {
        if (*seen == event) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD456A0.
void FTUEPersistentData::Reset() {
    mSeenEvents.clear();
}
