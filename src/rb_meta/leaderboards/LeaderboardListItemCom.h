#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

// The texts of one row of a song leaderboard list ("Component for items in
// the leaderboard list on song leaderboards."; author Kenny Sun; the object
// at 0xDB8740-0xDB937F, with the static initializer at 0xDB92C0, follows
// LeaderboardCom's). The leaderboard fills the properties and the row's
// scene shows them. Newer than the map, which has neither the object nor
// the class. The vtable at 0x1981980 has 41 slots. The object is 88 bytes.
class LeaderboardListItemCom : public Component {
public:
    // "row_type": what the row shows; the names are the allowed values'
    // texts. The enum's name is not in the reference map.
    enum RowType {
        kRowTypePrevious = 0,
        kRowTypeNext = 1,
        kRowTypeRank = 2,
        kRowTypePercentile = 3,
    };

    LeaderboardListItemCom();  // 0xDB8740
    // The imprint's copy starts with the defaults; _ImprintProps copies the
    // properties. Inlined into _Imprint (0xDB90F0). Not in the reference
    // map.
    LeaderboardListItemCom(const LeaderboardListItemCom& other);
    // Slots 0-1: 0xDB87D0, 0xDB87E0.
    ~LeaderboardListItemCom() override;

    Symbol GetId() const override;          // slot 4: 0xDB9070
    Symbol GetClassName() const override;   // slot 5: 0xDB9080
    int CurrentRev() const override;        // slot 7: 0xDB9090
    bool IsA(Symbol type) const override;   // slot 8: 0xDB90B0
    Component* AsComponent() override;      // slot 9: 0xDB90E0
    // Slot 10.
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xDB90F0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xDB9280
    ComMetaData& _GetMetaData() override;       // slot 23: 0xDB9290

    // Registers the class (InitComponentClass), emitted with the other
    // rb_meta classes' at 0xB64BE0.
    static void Init();
    // The class factory, emitted at 0xBA0D10; the placement factory beside
    // it (0xBA0D40) is InitComponentClass's.
    static Component* _Create();
    // Every property but "row_type" is stored and only records the
    // prototype's value as its default.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xDB8800

    // "LeaderboardListItemCom".
    static Symbol sId;  // 0x1B26590
    // Also "LeaderboardListItemCom".
    static Symbol sClassName;           // 0x1B26598
    static PropRegistry sPropRegistry;  // 0x1B265A0
    static ComMetaData sMetaData;       // 0x1B26640

    // Field names are not in the reference map; they follow the
    // properties. The binary starts the fields after the Component base's
    // tail padding, which the alignment reproduces.
    alignas(8) bool mFirstAmongFriends;  // "first_among_friends"
    Symbol mRank;             // "rank"
    Symbol mName;             // "name"
    Symbol mPercentage;       // "percentage"
    Symbol mDifficulty;       // "difficulty"
    int mScore;               // "score"
    // "full_combo", a float property.
    float mFullCombo;
    // "platfrom_uid" (sic): the player's platform id.
    Symbol mPlatformUid;
    // "row_type", read and set through accessors.
    RowType mRowType;
    bool mShowPercent;     // "show_percent"
    bool mShowDifficulty;  // "show_difficulty"
    bool mShowFullCombo;   // "show_full_combo"
    bool mIsYou;           // "is_you"
};

static_assert(offsetof(LeaderboardListItemCom, mFirstAmongFriends) == 24);
static_assert(offsetof(LeaderboardListItemCom, mRank) == 32);
static_assert(offsetof(LeaderboardListItemCom, mScore) == 64);
static_assert(offsetof(LeaderboardListItemCom, mFullCombo) == 68);
static_assert(offsetof(LeaderboardListItemCom, mPlatformUid) == 72);
static_assert(offsetof(LeaderboardListItemCom, mRowType) == 80);
static_assert(offsetof(LeaderboardListItemCom, mIsYou) == 87);
static_assert(sizeof(LeaderboardListItemCom) == 88);
