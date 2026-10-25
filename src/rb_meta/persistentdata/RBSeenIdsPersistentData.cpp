// RBSeenIdsPersistentData.o, newer than the map (0xD69080 to 0xD697EB). The
// set's subtree release is the shared eastl::set<Symbol> instance at
// 0x93CFE0. The static initializer at 0xD697C0 sets the shared header
// globals every rb_meta object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBSeenIdsPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reconstructed from eboot.elf at 0xD69080. Clear is inlined.
RBSeenIdsPersistentData::RBSeenIdsPersistentData(RBProfile* profile) : mProfile(profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD691B0.
void RBSeenIdsPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSeenIds.clear();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD69240. The deleting destructor is at
// 0xD69260.
RBSeenIdsPersistentData::~RBSeenIdsPersistentData() {}

// Reconstructed from eboot.elf at 0xD692A0.
const char* RBSeenIdsPersistentData::_GetProfileName() const {
    if (mProfile == nullptr) {
        return "NULL_PROFILE_POINTER!!!";
    }
    return mProfile->GetPlatformName();
}

// Reconstructed from eboot.elf at 0xD692C0.
void RBSeenIdsPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 0, 0);
        revisionStream.BeginWrite();
        int count = static_cast<int>(mSeenIds.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (const eastl::rbtree_node_base* node = mSeenIds.mAnchor.mpNodeLeft; node != &mSeenIds.mAnchor;
             node = eastl::RBTreeIncrement(node)) {
            revisionStream << static_cast<const eastl::set<Symbol>::node_type*>(node)->mValue;
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD693C0. A block that cannot be read
// only fetches the profile's name for a stripped log.
void RBSeenIdsPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 0, 0);
        if (revisionStream.BeginRead()) {
            unsigned int count;
            revisionStream.ReadEndian(&count, sizeof(count));
            for (unsigned int i = 0; i < count; ++i) {
                Symbol id;
                revisionStream >> id;
                mSeenIds.insert(id);
            }
        } else {
            static_cast<void>(_GetProfileName());
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD695D0.
bool RBSeenIdsPersistentData::HasSeenId(Symbol id) const {
    return mSeenIds.find(id) != mSeenIds.end();
}

// Reconstructed from eboot.elf at 0xD69640.
bool RBSeenIdsPersistentData::MarkIdSeen(Symbol id) {
    if (HasSeenId(id)) {
        return false;
    }
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSeenIds.insert(id);
    MakeDirty();
    MemPopHeap();
    return true;
}
