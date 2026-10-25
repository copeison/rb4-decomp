#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "os/threading/CritSec.h"
#include "os/users/UserGuid.h"
#include "persistence/core/PersistentData.h"
#include "rb_world/characters/RBCharOutfitDesc.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BandUser;
class BinStream;
class RBProfile;

// A profile's characters (rb_meta/RBCharacterPersistentData.o,
// 0xD4D520-0xD4FC1F): the characters it created (at most 16), the character
// it saved last and which band users use which character. Everything but
// Save and the accessors of the saved character runs under mCritSec. The
// vtable at 0x197D7A8 has PersistentData's 7 slots; the object is 128
// bytes. The profile keeps one (RBProfile::mCharacterData) and loads it
// through Revision::AsOfOp<RBCharacterPersistentData>.
class RBCharacterPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    // Which user uses one of the characters. The map names the type.
    // Field names not in the reference map.
    struct CharUseInfo {
        Symbol mCharName;
        bool mInUse;
        UserGuid mUserGuid;
    };

    // The most characters a profile keeps; the vectors reserve as many.
    // Name not in the reference map.
    static constexpr unsigned long kMaxCharacters = 16;

    explicit RBCharacterPersistentData(RBProfile* profile);  // 0xD4D520
    // Slots 0-1: 0xD4D610, 0xD4D6C0.
    ~RBCharacterPersistentData() override;

    // Slot 5: the count, the saved character and the characters in a
    // revision-4 block.
    void Save(BinStream& stream) const override;  // 0xD4D890
    // Slot 6: blocks before revision 3 leave the characters empty; the
    // saved character comes from revision 2.
    void Load(BinStream& stream) override;  // 0xD4D9E0

    // The profile's platform name for logs, or "NULL_PROFILE_POINTER!!!".
    const char* _GetProfileName() const;  // 0xD4D6E0
    // Forgets the characters, the saved character and the users; does not
    // mark the data dirty.
    void Clear();  // 0xD4D700

    // Rebuilds the use entries, one free entry per character.
    void _CreateCharUseVecAfterLoad();  // 0xD4DDD0
    void GetCharacterNames(eastl::vector<Symbol>& names);  // 0xD4DEE0
    void GetCharacters(eastl::vector<RBChar::RBCharDesc>& characters);  // 0xD4E0D0
    bool HasRoomForMoreCharacters();  // 0xD4E220
    unsigned long GetCharacterCount();  // 0xD4E280
    // The character of that name, or the end of the characters.
    const RBChar::RBCharDesc* _FindCharacterByName(Symbol name) const;  // 0xD4E2D0
    RBChar::RBCharDesc* _FindCharacterByNameMutable(Symbol name);  // 0xD4E300
    bool CharacterExists(Symbol name);  // 0xD4E330
    // Copies the character, or a default description when there is none.
    bool GetCharacterByName(Symbol name, RBChar::RBCharDesc& desc);  // 0xD4E3B0
    // Adds the description, or a default one with the name when it is
    // null. Fails when the name is taken or the profile is full.
    bool CreateCharacter(Symbol name, RBChar::RBCharDesc* desc);  // 0xD4E520
    // Reports named default pieces outside the instrument slots; it only
    // logs.
    void _CheckForDefaultPieces(const RBChar::RBCharDesc& desc);  // 0xD4E790
    // Replaces the character's use entry with a free one.
    void _CreateCharUseEntryForChar(const Symbol& name);  // 0xD4E870
    // Declared only: it sends the outfit change to RBCampaignMgr
    // (SendCharacterOutfitUpdateBI at 0x95A380 on the object at 0x1AE03C0),
    // which is not modelled. It replaces the description and marks the
    // data dirty.
    bool UpdateCharacter(Symbol name, const RBChar::RBCharDesc& desc);  // 0xD4EAB0
    // Renames the character, taking the description from `desc` when it
    // is set, and moves its use entry to a free one under the new name.
    bool RenameCharacter(Symbol oldName, Symbol newName, const RBChar::RBCharDesc* desc);  // 0xD4EBF0
    void _DestroyCharUseEntryForChar(const Symbol& name);  // 0xD4EE60
    bool DeleteCharacter(Symbol name);  // 0xD4EF00
    Symbol GetSavedCharName(bool& isPrefab) const;  // 0xD4F080
    Symbol GetSavedCharName() const;  // 0xD4F090
    // Records the character, a prefab of the character manager when
    // `isPrefab` is set, otherwise one of the profile's. The empty name is
    // stored as a prefab.
    bool SetSavedCharName(Symbol name, bool isPrefab);  // 0xD4F0A0
    // True also for an unknown character.
    bool CharIsInUse(const Symbol& name);  // 0xD4F1D0
    CharUseInfo* _GetCharUseInfoForChar(const Symbol& name);  // 0xD4F260
    bool TryToMarkCharInUse(const Symbol& name, const UserGuid& user);  // 0xD4F290
    bool TryToGetFirstNotInUseCharAndMarkIt(const UserGuid& user, Symbol& name);  // 0xD4F370
    void ClearCharInUse(const Symbol& name, const UserGuid& user);  // 0xD4F440
    void ClearAllCharsInUseForUser(const UserGuid& user);  // 0xD4F5B0
    bool UserIsUsingChar(const UserGuid& user, const Symbol& name);  // 0xD4F6D0
    // Declared only: these read BandUser's character state (+0x68 name,
    // +0x70 type, +0x74 owner; GetCurrentCharName 0x9AA8E0,
    // IsUsingProfileChar 0x9AA910, a newer owner-aware check 0x9AA930) and
    // the band user manager's GetBandUserByProfile (0x9248F0 on the object
    // at 0x1ADFB18), which are not modelled. CheckCharUsage frees entries
    // no user uses and calls slot 13 of each user found using a free one.
    void CheckCharUsage();  // 0xD4F760
    // The map's build returns a bool; this one returns the user.
    BandUser* _IsUserUsingChar(const eastl::vector<BandUser*>& users, const Symbol& name,
                               const BandUser* owner) const;  // 0xD4F8F0

    // Field names are not in the reference map.
    RBProfile* mProfile;
    eastl::vector<RBChar::RBCharDesc> mCharacters;
    Symbol mSavedCharName;
    bool mSavedCharIsPrefab;
    eastl::vector<CharUseInfo> mCharUseInfo;
    CritSec mCritSec;
};

static_assert(offsetof(RBCharacterPersistentData::CharUseInfo, mInUse) == 8);
static_assert(offsetof(RBCharacterPersistentData::CharUseInfo, mUserGuid) == 12);
static_assert(sizeof(RBCharacterPersistentData::CharUseInfo) == 32);
static_assert(offsetof(RBCharacterPersistentData, mProfile) == 24);
static_assert(offsetof(RBCharacterPersistentData, mCharacters) == 32);
static_assert(offsetof(RBCharacterPersistentData, mSavedCharName) == 64);
static_assert(offsetof(RBCharacterPersistentData, mSavedCharIsPrefab) == 72);
static_assert(offsetof(RBCharacterPersistentData, mCharUseInfo) == 80);
static_assert(offsetof(RBCharacterPersistentData, mCritSec) == 112);
static_assert(sizeof(RBCharacterPersistentData) == 128);
