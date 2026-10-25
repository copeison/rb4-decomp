#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "os/threading/CritSec.h"
#include "os/users/HxGuid.h"
#include "persistence/core/PersistentData.h"
#include "utl/containers/Vector.h"

class BinStream;
class RBProfile;

// One saved setlist (vtable 0x198E7E0, code at 0xEF17B0-0xEF1D40 near the
// "setlist_name", "setlist_guid" and "setlist_song_count" strings): the
// profile, the name, the songs and a GUID that identifies it. Only the
// members RBSetlistPersistentData uses are declared; the class is not
// reconstructed, and it is declared here because no header models it. Name
// not in the reference map; the evidence for it is weak.
class RBSetlist : public PersistentData {
public:
    RBSetlist();                                 // 0xEF17B0
    RBSetlist(const RBSetlist& other);           // 0xEF19F0
    // Copies everything but the parent and the profile, and marks the
    // setlist dirty.
    RBSetlist& operator=(const RBSetlist& other);  // 0xEF1A70
    // Slots 0-1: 0xEF1BA0, 0xEF1BD0.
    ~RBSetlist() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xEF1C10
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xEF1D40

    // Field names are not in the reference map. The name symbol (+32),
    // two values, the song id vector (+48) are not reconstructed.
    RBProfile* mProfile;
    unsigned char mSetlistContents[80 - 32];
    HxGuid mGuid;
};

static_assert(offsetof(RBSetlist, mProfile) == 24);
static_assert(offsetof(RBSetlist, mGuid) == 80);
static_assert(sizeof(RBSetlist) == 96);

// A profile's saved setlists, newer than the map (its object is at
// 0xD650F0-0xD65CAF, after RBRockShopPersistentData.o): a vector that the
// setlist screens (RBSetlistListProviderCom, "setlist_changed") read and
// edit, with a lock that only saving and loading take. The vtable at
// 0x197E028 has PersistentData's 7 slots; the object is 80 bytes. Name not
// in the reference map; the evidence for it is weak.
class RBSetlistPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBSetlistPersistentData(RBProfile* profile);  // 0xD650F0
    // Slots 0-1: 0xD651A0, 0xD65250.
    ~RBSetlistPersistentData() override;

    // Slot 5: the setlists in a revision-0 block.
    void Save(BinStream& stream) const override;  // 0xD65290
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD653D0

    // The profile's platform name for logs. Name not in the reference map.
    const char* _GetProfileName() const;  // 0xD65270
    // Forgets the setlists without marking the data dirty. Name not in the
    // reference map.
    void Clear();  // 0xD65530
    // The setlist at the index, or null. Name not in the reference map.
    RBSetlist* GetSetlist(unsigned long index);  // 0xD655C0
    // Appends a copy owned by this data. Name not in the reference map.
    void AddSetlist(const RBSetlist& setlist);  // 0xD65600
    // Removes the setlist at the index; false when there is none. Name not
    // in the reference map.
    bool RemoveSetlist(unsigned long index);  // 0xD656A0
    // Overwrites the setlist at the index, when there is one. Name not in
    // the reference map.
    void ReplaceSetlist(unsigned long index, const RBSetlist& setlist);  // 0xD657A0

    // Field names are not in the reference map.
    RBProfile* mProfile;
    mutable CritSec mLock;
    eastl::vector<RBSetlist> mSetlists;
};

static_assert(offsetof(RBSetlistPersistentData, mProfile) == 24);
static_assert(offsetof(RBSetlistPersistentData, mLock) == 32);
static_assert(offsetof(RBSetlistPersistentData, mSetlists) == 48);
static_assert(sizeof(RBSetlistPersistentData) == 80);
