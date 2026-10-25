#pragma once

#include <cstddef>

#include "os/users/UserGuid.h"
#include "rb_game/users/BandUser.h"
#include "rb_world/characters/RBCharOutfitDesc.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// The game's band user (0x9A9A00-0x9AC060 with inline members emitted next
// to the local and remote users): the common base of LocalBandUser and
// RemoteBandUser. It holds what the map's BandUser kept about the user's
// character (the preview and current character, each a name, a type and
// the owner's id) and the user's flags in the online (SOMP) session, and
// adds the user's preference and session hooks (slots 24-42). Newer than
// the map; the class name is not in the reference map and the evidence for
// it is weak. The vtable at 0x1962360 has 43 slots (slots 12, 19, 21,
// 24-26, 29, 41 and 42 pure); the object is 232 bytes. The character
// methods keep the map's BandUser names; the SOMP request steps are new.
// The binary keeps the vector<Symbol> assignment it uses at 0x9AC050.
class RBBandUser : public BandUser {
public:
    // Which kind of character a character name refers to. The map names the
    // type (BandUser::CharType); only the "none" value is recovered. Name of
    // the enumerator not in the reference map.
    // Enumerator names not in the reference map.
    enum CharType : int {
        kCharTypeNone = -1,
        // One of the user's profile's characters.
        kCharTypeProfile = 0,
        // A prefab character of the character manager.
        kCharTypePrefab = 1,
        // A character of another user's profile; the owner is that user.
        kCharTypeOtherProfile = 2,
    };

    // The steps of the user's requests to the online session (+0x8C).
    // States 0-3 are not used by the band user code. Names not in the
    // reference map.
    enum SompRequestState {
        kSompRequestIdle = 4,
        kSompPartsRequested = 5,
        kSompPartsReceived = 6,
        kSompPartRequested = 7,
        kSompPartGranted = 8,
        kSompDifficultyRequested = 9,
        kSompDifficultyGranted = 10,
    };

    RBBandUser();  // 0x9A9A00
    // Slots 0-1: 0x9ABF70, 0x9ABFE0.
    ~RBBandUser() override;
    // Slot 4: the profile's pad, else the user's own platform user id.
    int GetPlatformUserId() const override;  // 0x9A9AE0
    // Slots 9-11. Inline; the copies kept are at 0x92B870, 0x92B880 and
    // 0x92B890.
    bool IsSompHost() const override {
        return mSompHost;
    }
    bool IsSompLeader() const override {
        return mSompLeader;
    }
    bool IsSompReady() const override {
        return mSompReady;
    }
    // Slot 13: the profile's platform name, else the localized "guest".
    const char* GetPlatformName() const override;  // 0x9A9FD0
    // Slot 18: clears both characters and the character lists.
    void _Reset() override;  // 0x9A9B10
    // Slot 20: the profile's achievement data's logger.
    IRBAchievementLogger* GetAchievementData() const override;  // 0x9A9F90
    // Slot 22: on the guitar part, sets the in-game sound VCA to the
    // options' volume.
    void _ApplyTrackVolumeSettings() override;  // 0x9A9CA0

    // Slot 24: whether the user plays on this console.
    virtual bool IsLocal() const = 0;
    // Slots 25-28: the user's preferred symbol and index, kept in the
    // profile's RBUserPreferencesPersistentData (mSymbolValue, mLongValue)
    // or, for a remote user, read from the online session. What they
    // select is not recovered; names not in the reference map, and the
    // evidence is weak.
    virtual Symbol GetPreferenceSymbol() const = 0;
    virtual long GetPreferenceIndex() const = 0;
    // Slots 27-28: empty here.
    virtual void SetPreferenceSymbol(Symbol value) {  // 0x9A5830
        static_cast<void>(value);
    }
    virtual void SetPreferenceIndex(long value) {  // 0x9A5840
        static_cast<void>(value);
    }
    // Slots 29-34: three of the preferences' four flags (mFlags[0], [2]
    // and [3]) and their setters. Names not in the reference map; the
    // evidence is weak.
    virtual bool GetPreferenceFlag0() const = 0;
    virtual void SetPreferenceFlag0(bool value) {  // 0x9A5850
        static_cast<void>(value);
    }
    virtual bool GetPreferenceFlag2() const {  // 0x9A5860
        return false;
    }
    virtual void SetPreferenceFlag2(bool value) {  // 0x9A5870
        static_cast<void>(value);
    }
    virtual bool GetPreferenceFlag3() const {  // 0x9A5880
        return false;
    }
    virtual void SetPreferenceFlag3(bool value) {  // 0x9A5890
        static_cast<void>(value);
    }
    // Slots 35-38: a second symbol and index the user picked (a local user
    // picks the symbol from a list that is not identified). Names not in
    // the reference map; the evidence is weak.
    virtual Symbol GetPickedSymbol() const {  // 0x9A58A0
        return Symbol();
    }
    virtual long GetPickedIndex() const {  // 0x9A58B0
        return -1;
    }
    virtual void PickSymbol() {}  // 0x9A58C0
    virtual void SetPickedIndex(long value) {  // 0x9A58D0
        static_cast<void>(value);
    }
    // Slots 39-40: tell the online session about the user and about its
    // preferences. Names not in the reference map; the evidence is weak.
    virtual void _UpdateSompMember() {}  // 0x9A58E0
    virtual void _SendSompPreferences() {}  // 0x9A58F0
    // Slots 41-42: online session queries. The local user's slot 41 tells
    // whether a session value is above 500 (likely a delay in
    // milliseconds) and its slot 42 is false; a remote user answers two
    // flags of its session record. Names not in the reference map; the
    // evidence is weak.
    virtual bool IsSompLagging() const = 0;
    virtual bool IsSompIdle() const = 0;

    // Sets the current character without locking it: the name, the type
    // and the owner (a null owner gives a null id), then the profile's
    // saved character (the map's _UpdateProfileCurrentCharIfOwner,
    // inlined) and the session (slot 39). Name not in the reference map;
    // the map's SetCurrentCharName is the locking setter below.
    void _SetCurrentCharName(const Symbol& name, const CharType& type, const UserGuid* owner);  // 0x9A9BB0
    // Applies the vocal stem and microphone volumes of the options to the
    // game's vocal mixer and microphone channels; in a game whose data asks
    // for it the microphones are silenced. The map has
    // BandUser::ApplyVocalistVolumeSettings() const; this build takes a
    // flag. Not reconstructed: it calls the game (theGame 0x1AEF770, the
    // mutable game's accessor 0xA48560 and its slot 37) and the game data
    // (0x965DF0), which are not modelled.
    void ApplyVocalistVolumeSettings(bool inGame);  // 0x9A9D00
    // Whether a local user's connected instrument is not the controller the
    // user plays. Name not in the reference map.
    bool IsUsingOtherController() const;  // 0x9A9F50

    // The preview character's description; false (and the default
    // description) without a preview character or when it is missing.
    bool GetPreviewCharacterDesc(RBChar::RBCharDesc& desc) const;  // 0x9AA110
    // The description of a character: a profile character from the user's
    // profile, another user's from that user's profile, a prefab from the
    // character manager. False, with the default description, when it is
    // missing.
    bool _GetCharDesc(const Symbol& name, const CharType& type, const UserGuid& owner,
                      RBChar::RBCharDesc& desc) const;  // 0x9AA250
    // Sets the preview character when it exists; the empty name clears it.
    // False when the character is missing (the preview is then cleared).
    bool SetPreviewCharName(const Symbol& name, const CharType& type, const UserGuid* owner);  // 0x9AA6D0
    // Whether the character exists: in the user's profile, in the owner's
    // profile, or among the prefabs.
    bool _VerifyExistenceOfCharacter(const Symbol& name, const CharType& type,
                                     const UserGuid* owner) const;  // 0x9AA7F0
    Symbol GetCurrentCharName(CharType& type, UserGuid& owner) const;  // 0x9AA8C0
    Symbol GetCurrentCharName() const;  // 0x9AA8E0
    bool IsUsingPrefabChar(const Symbol& name) const;  // 0x9AA8F0
    bool IsUsingProfileChar(const Symbol& name) const;  // 0x9AA910
    // Whether the user uses the owner's profile character.
    bool IsUsingOtherProfileChar(const Symbol& name, const UserGuid& owner) const;  // 0x9AA930
    bool GetCurrentCharacterDesc(RBChar::RBCharDesc& desc) const;  // 0x9AA950
    // Clears the preview, releases the current character and locks the new
    // one; the empty name only releases. False when the character cannot
    // be locked (the user is then left without one).
    bool SetCurrentCharName(const Symbol& name, const CharType& type, const UserGuid* owner);  // 0x9AAA90
    // Releases the lock on the current character.
    void _ClearCurrentCharUsage();  // 0x9AAC00
    // Locks the character for the user: in the user's profile, in the
    // owner's profile or among the prefabs. False when it is missing or
    // used.
    bool _TryToLockCharacter(const Symbol& name, const CharType& type, const UserGuid& owner);  // 0x9AACC0
    // Clears the preview and takes a random free prefab; false without one.
    bool SetRandomPrefabChar();  // 0x9AAD90
    // Drops the current character when the user no longer holds its lock.
    void CheckCharacterUsage();  // 0x9AAE70
    // Clears the current character without releasing it.
    void _ForceClearCurrentCharUsage();  // 0x9AB010
    // Saves the current character as the profile's when the user's slot
    // owns the profile's settings; another user's character is not saved.
    void _UpdateProfileCurrentCharIfOwner();  // 0x9AB070
    // Pushes the character to the stage; false without a character
    // manager.
    bool PushCurrentCharDescToEntity();  // 0x9AB100
    // Takes the profile's first free character, else (when allowed) a
    // random prefab, else none, and pushes it to the stage.
    void SwitchCurrentCharToExistingChar(const bool& allowRandomPrefab);  // 0x9AB160
    // Leaves another user's character for one of the user's own.
    void SwitchAwayFromOtherProfileCharIfNeeded();  // 0x9AB250
    bool HasAnyProfileChars();  // 0x9AB2D0
    bool HasProfileCharWithName(const Symbol& name);  // 0x9AB310
    // Whether the Rock Shop has pieces for the slot and gender that the
    // profile has not viewed.
    bool HasUnviewedPiecesForSlot(const RBChar::RBCharOutfitSlot& slot,
                                  const RBChar::RBCharGender& gender) const;  // 0x9AB350
    bool HasAnyUnviewedItems() const;  // 0x9AB6A0
    // The tops, bottoms and glasses; the hair, facial hair and full face
    // make-up; the guitar, bass and drums.
    bool HasUnviewedClothing(const RBChar::RBCharGender& gender) const;  // 0x9AB6D0
    bool HasUnviewedHairOrMakeup(const RBChar::RBCharGender& gender) const;  // 0x9AB770
    bool HasUnviewedInstruments(const RBChar::RBCharGender& gender) const;  // 0x9AB810
    // The user's online id as text, else the platform name; the SOMP
    // client sends it as the member's name. Name not in the reference map;
    // the evidence is weak.
    Symbol GetOnlineName() const;  // 0x9AB8B0

    // The user's requests to the online (SOMP) session in the song settings
    // flow. Names not in the reference map.
    int GetSompRequestState() const;  // 0x9AB960
    void SetSompRequestState(int state);  // 0x9AB970
    eastl::vector<Symbol>& GetSompParts();  // 0x9AB980
    eastl::vector<int>& GetSompPartStates();  // 0x9AB990
    // Back to kSompRequestIdle with no parts.
    void ResetSompRequests();  // 0x9AB9A0
    // Asks the session for the parts the user may choose. Not
    // reconstructed: it calls the SOMP client (0x98AFB0, 0x98AA40), whose
    // members are not declared in RBSompSessionMgr.h.
    void RequestSompParts();  // 0x9AB9D0
    // Takes the parts the session sent: the answer to the request
    // (`requested`) or a later change.
    void SetSompParts(const eastl::vector<Symbol>& parts, const eastl::vector<int>& partStates,
                      bool requested);  // 0x9ABA70
    // Asks the session for the part. Not reconstructed: it calls the SOMP
    // client (0x98AAD0).
    void RequestSompPart(int part);  // 0x9ABBF0
    // Takes the part the session granted (guitar, bass, drums, pro drums,
    // vocals or harmonies): the track and the score type, when the user's
    // controller and slot fit it.
    void SetSompPart(int part);  // 0x9ABC70
    // Asks the session for the difficulty. Not reconstructed: it calls the
    // SOMP client (0x98AB70).
    void RequestSompDifficulty(Difficulty difficulty);  // 0x9ABDD0
    // Takes the difficulty the session granted.
    void SetSompDifficulty(Difficulty difficulty);  // 0x9ABE50
    // The preference symbol and index to send to the session: the picked
    // ones when the first and third flags ask for them. Names not in the
    // reference map; the evidence is weak.
    Symbol GetSompPreferenceSymbol() const;  // 0x9ABEB0
    long GetSompPreferenceIndex() const;  // 0x9ABF00

    // Field names are not in the reference map.
    Symbol mPreviewCharName;
    CharType mPreviewCharType;
    UserGuid mPreviewCharOwner;
    Symbol mCurrentCharName;
    CharType mCurrentCharType;
    UserGuid mCurrentCharOwner;
    // A value and two bytes the reconstructed code does not read; the
    // names are weak.
    int mCharUsage;
    unsigned char mCharUsageFlags[2];
    // The SompRequestState; _Reset sets -1.
    int mSompRequestState;
    // Two flags _Reset clears; not read by the reconstructed code.
    bool mCharFlags[2];
    // The session flags of slots 9-11; _Reset clears them and the band
    // user manager copies them from the session's member table.
    bool mSompHost;
    bool mSompLeader;
    bool mSompReady;
    // Four ids that start at -1; the overshell reads the first. The name
    // is weak.
    int mCharIds[4];
    // The parts the session lets the user choose and their states.
    eastl::vector<Symbol> mSompParts;
    eastl::vector<int> mSompPartStates;
};

static_assert(offsetof(RBBandUser, mPreviewCharName) == 72);
static_assert(offsetof(RBBandUser, mPreviewCharType) == 80);
static_assert(offsetof(RBBandUser, mPreviewCharOwner) == 84);
static_assert(offsetof(RBBandUser, mCurrentCharName) == 104);
static_assert(offsetof(RBBandUser, mCurrentCharType) == 112);
static_assert(offsetof(RBBandUser, mCurrentCharOwner) == 116);
static_assert(offsetof(RBBandUser, mCharUsage) == 132);
static_assert(offsetof(RBBandUser, mSompRequestState) == 140);
static_assert(offsetof(RBBandUser, mSompHost) == 146);
static_assert(offsetof(RBBandUser, mSompReady) == 148);
static_assert(offsetof(RBBandUser, mCharIds) == 152);
static_assert(offsetof(RBBandUser, mSompParts) == 168);
static_assert(offsetof(RBBandUser, mSompPartStates) == 200);
static_assert(sizeof(RBBandUser) == 232);
