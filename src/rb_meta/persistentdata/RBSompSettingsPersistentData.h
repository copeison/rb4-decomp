#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// The settings of an online (SOMP) session of the social multiplayer code
// in rb_game: eight enumerated choices and eight flags, among them the
// session's privacy ("PublicSession", "FriendsOnlySession", ...), the
// setlist type, order and length and the gameplay modifiers ("NoFail",
// "BrutalMode", "SuperMode"), whose string tables are next to its code at
// 0xA82840-0xA83680. Which field holds which choice is not known. Only the
// members RBSompSettingsPersistentData uses are declared; the struct is not
// reconstructed, and it is declared here because no rb_game header models
// it. Names not in the reference map; the evidence for them is weak.
struct SompSessionSettings {
    // Every choice -1 and every flag false.
    SompSessionSettings();  // 0xA835F0

    int mFirstChoices[4];
    bool mFlags[8];
    int mLastChoices[4];
};

static_assert(offsetof(SompSessionSettings, mFlags) == 16);
static_assert(offsetof(SompSessionSettings, mLastChoices) == 24);
static_assert(sizeof(SompSessionSettings) == 40);

// The defaults: every choice 0, the first flag set. Each rb_meta object's
// static initializer builds its copy of a shared header's defaults global
// with it. Name not in the reference map.
SompSessionSettings GetDefaultSompSessionSettings();  // 0xA82840

// The online session settings a profile used last, newer than the map (its
// object is at 0xD65CB0-0xD663EF, before RBSystemOptions.o). The vtable at
// 0x197E098 has PersistentData's 7 slots; the object is 64 bytes and has no
// profile pointer of its own. Name not in the reference map; the evidence
// for it is weak.
class RBSompSettingsPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBSompSettingsPersistentData(RBProfile* profile);  // 0xD65CB0
    // Slots 0-1: 0xD66390, 0xD663A0. The profile's destructor does not call
    // it: its body is empty.
    ~RBSompSettingsPersistentData() override;

    // Slot 5: the choices, then the flags, in a revision-1 block.
    void Save(BinStream& stream) const override;  // 0xD65D20
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD65F40

    // Restores the default settings without marking the data dirty. Name
    // not in the reference map.
    void Clear();  // 0xD65D00

    SompSessionSettings mSettings;  // Name not in the reference map.
};

static_assert(offsetof(RBSompSettingsPersistentData, mSettings) == 24);
static_assert(sizeof(RBSompSettingsPersistentData) == 64);
