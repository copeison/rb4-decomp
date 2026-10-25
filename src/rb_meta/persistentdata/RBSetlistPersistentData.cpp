// RBSetlistPersistentData.o, newer than the map (0xD650F0 to 0xD65CAF).
// The object also emits Revision::AsOfOp<eastl::vector<RBSetlist>> (vtable
// 0x197E070: destructors 0xD65520 and 0xD65870, Read 0xD65880) and the
// vector's growth helpers (0xD65990 for resize, 0xD65B50 for push_back).
// The static initializer at 0xD65C70 sets the shared header globals every
// rb_meta object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBSetlistPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reads a count and loads that many setlists, as the AsOfOp's Read
// (0xD65880) inlines it. Name not in the reference map; the original is
// probably a generic vector reader.
static BinStream& operator>>(BinStream& stream, eastl::vector<RBSetlist>& setlists) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    setlists.resize(count);
    for (RBSetlist* setlist = setlists.begin(); setlist != setlists.end(); ++setlist) {
        setlist->Load(stream);
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xD650F0. The lock is recursive and named
// "hx crit sec".
RBSetlistPersistentData::RBSetlistPersistentData(RBProfile* profile) : mProfile(profile) {
    mParent = profile;
}

// Reconstructed from eboot.elf at 0xD651A0. The deleting destructor is at
// 0xD65250.
RBSetlistPersistentData::~RBSetlistPersistentData() {}

// Reconstructed from eboot.elf at 0xD65270.
const char* RBSetlistPersistentData::_GetProfileName() const {
    if (mProfile == nullptr) {
        return "NULL_PROFILE_POINTER!!!";
    }
    return mProfile->GetPlatformName();
}

// Reconstructed from eboot.elf at 0xD65290.
void RBSetlistPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        ScopedCritSec lock(mLock);
        RevisionStream revisionStream(stream, 0, 0);
        revisionStream.BeginWrite();
        unsigned int count = static_cast<unsigned int>(mSetlists.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (const RBSetlist* setlist = mSetlists.begin(); setlist != mSetlists.end(); ++setlist) {
            setlist->Save(revisionStream);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD653D0. The loaded setlists are
// re-pointed at this data and the profile; a block that cannot be read only
// fetches the profile's name for a stripped log.
void RBSetlistPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        ScopedCritSec lock(mLock);
        RevisionStream revisionStream(stream, 0, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<eastl::vector<RBSetlist>>(&mSetlists, 0);
            for (RBSetlist* setlist = mSetlists.begin(); setlist != mSetlists.end(); ++setlist) {
                setlist->mParent = this;
                setlist->mProfile = mProfile;
            }
        } else {
            static_cast<void>(_GetProfileName());
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD65530.
void RBSetlistPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSetlists.clear();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD655C0.
RBSetlist* RBSetlistPersistentData::GetSetlist(unsigned long index) {
    if (index < mSetlists.size()) {
        return &mSetlists[index];
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xD65600.
void RBSetlistPersistentData::AddSetlist(const RBSetlist& setlist) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSetlists.push_back(setlist);
    RBSetlist& added = mSetlists.back();
    added.mParent = this;
    added.mProfile = mProfile;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD656A0.
bool RBSetlistPersistentData::RemoveSetlist(unsigned long index) {
    if (index >= mSetlists.size()) {
        return false;
    }
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSetlists.erase(mSetlists.begin() + index);
    MakeDirty();
    MemPopHeap();
    return true;
}

// Reconstructed from eboot.elf at 0xD657A0. The comparison of the GUIDs is
// what remains of a stripped assert.
void RBSetlistPersistentData::ReplaceSetlist(unsigned long index, const RBSetlist& setlist) {
    if (index >= mSetlists.size()) {
        return;
    }
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    static_cast<void>(setlist.mGuid == mSetlists[index].mGuid);
    mSetlists[index] = setlist;
    MakeDirty();
    MemPopHeap();
}
