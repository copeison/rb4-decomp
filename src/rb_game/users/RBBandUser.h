#pragma once

#include <cstddef>

#include "os/users/UserGuid.h"
#include "rb_game/users/BandUser.h"
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
// 24-26, 29, 41 and 42 pure); the object is 232 bytes.
class RBBandUser : public BandUser {
public:
    // Which kind of character a character name refers to. The map names the
    // type (BandUser::CharType); only the "none" value is recovered. Name of
    // the enumerator not in the reference map.
    enum CharType : int {
        kCharTypeNone = -1,
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
    // Slot 20.
    void* GetAchievementData() const override;  // 0x9A9F90
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

    // Sets the current character; a null owner gives the user a new id.
    // Then saves the choice in the slot's profile and asks the derived
    // class to tell the session. The map's BandUser::SetCurrentCharName;
    // not reconstructed.
    void SetCurrentCharName(const Symbol& name, const CharType& type, const UserGuid* owner);  // 0x9A9BB0
    // Applies the vocal stem and microphone volumes of the options to the
    // band's vocal mixers. The map has BandUser::ApplyVocalistVolumeSettings()
    // const; this build takes a flag. Not reconstructed.
    void ApplyVocalistVolumeSettings(bool inGame);  // 0x9A9D00
    // Whether a local user's connected instrument is not the controller the
    // user plays. Name not in the reference map.
    bool IsUsingOtherController() const;  // 0x9A9F50
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
    // The character bookkeeping of the map's character functions, which
    // the reconstructed code does not read: a value and two bytes, a
    // loading state that steps from 4 to 10 (0x9AB960-0x9ABE80) and that
    // _Reset sets to -1, two flags, and four ids that start at -1. The
    // names follow those functions; the evidence is weak.
    int mCharUsage;
    unsigned char mCharUsageFlags[2];
    int mCharLoadState;
    bool mCharFlags[2];
    // The session flags of slots 9-11; _Reset clears them.
    bool mSompHost;
    bool mSompLeader;
    bool mSompReady;
    int mCharIds[4];
    // Two lists _Reset empties; their element types are not recovered.
    eastl::vector<unsigned char> mCharLists[2];
};

static_assert(offsetof(RBBandUser, mPreviewCharName) == 72);
static_assert(offsetof(RBBandUser, mPreviewCharType) == 80);
static_assert(offsetof(RBBandUser, mPreviewCharOwner) == 84);
static_assert(offsetof(RBBandUser, mCurrentCharName) == 104);
static_assert(offsetof(RBBandUser, mCurrentCharType) == 112);
static_assert(offsetof(RBBandUser, mCurrentCharOwner) == 116);
static_assert(offsetof(RBBandUser, mCharUsage) == 132);
static_assert(offsetof(RBBandUser, mCharLoadState) == 140);
static_assert(offsetof(RBBandUser, mSompHost) == 146);
static_assert(offsetof(RBBandUser, mSompReady) == 148);
static_assert(offsetof(RBBandUser, mCharIds) == 152);
static_assert(offsetof(RBBandUser, mCharLists) == 168);
static_assert(sizeof(RBBandUser) == 232);
