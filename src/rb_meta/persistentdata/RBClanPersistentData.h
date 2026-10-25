#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "rb_meta/clans/RBSpotlightHighScore.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"

class BinStream;
class RBProfile;

// A profile's cached Rivals clan state, newer than the map (its object is
// at 0xD4FC20-0xD52AEB, between RBCharacterPersistentData.o and
// RBGameplayOptions.o): the clan values the player last saw, which the
// clan screens compare with the current ones for their "has changed"
// properties (the setters run when the screens clear them), and the clan's
// spotlight song scores. The vtable at 0x197D7F0 has PersistentData's 7
// slots; the object is 136 bytes. The class, method and field names are
// not in the reference map; they follow the clan screens' property names
// ("clan_tier_cached", "clan_current_xp_rivals_points_cached", ...), and
// the evidence for the class name is weak.
class RBClanPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    // The revision Save writes. Name not in the reference map.
    static constexpr int kRevision = 6;

    explicit RBClanPersistentData(RBProfile* profile);  // 0xD4FC20
    // Slots 0-1: 0xD4FD80, 0xD4FDA0.
    ~RBClanPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD4FE00
    // Slot 6: the spotlight scores of revisions 1-4 and 5 are converted
    // from their older layouts.
    void Load(BinStream& stream) override;  // 0xD500B0

    // Restores the defaults without marking the data dirty: no seen event
    // (-1), tier 6, no spotlight event and no scores. The constructor,
    // RBRivalsPlayerProfile (0xB2564E) and RBProfile::DeleteAll call it.
    void Clear();  // 0xD4FCD0
    // The profile's platform name for logs, or "NULL_PROFILE_POINTER!!!".
    // Nothing calls it.
    const char* _GetProfileName() const;  // 0xD4FDE0

    // Each stores the value under the "profile" heap and marks the data
    // dirty.
    void SetSeenEventId(int id);                          // 0xD50FD0
    void SetCachedLifetimeRivalsPoints(int points);       // 0xD51040
    void SetCachedSeasonalRivalsPoints(int points);       // 0xD510B0
    void SetCachedTier(int tier);                         // 0xD51120
    void SetBestTier(int tier);                           // 0xD51190
    void SetSpotlightScoresEventId(int id);               // 0xD51200
    // Nothing calls it.
    void SetCachedPointsEventId(int id);                  // 0xD51270
    void SetCachedSpotlightRivalsPoints(int points);      // 0xD512E0
    void SetCachedXpRivalsPoints(int points);             // 0xD51350
    void SetCachedRankPointsRivalsPoints(int points);     // 0xD513C0
    void SetCachedCurrentRivalsPoints(int points);        // 0xD51430

    RBProfile* mProfile;
    // Set from the clan manager (+0x580) when the player dismisses the
    // event results. The name is weak.
    int mSeenEventId;
    int mCachedLifetimeRivalsPoints;  // "clan_lifetime_rivals_points"
    int mCachedSeasonalRivalsPoints;  // "clan_seasonal_rivals_points"
    int mCachedTier;                  // "clan_tier"
    // Lowered to the cached tier when that one is lower. The name is weak.
    int mBestTier;
    // The spotlight event the scores belong to; the spotlight stats job
    // clears the scores when it changes.
    int mSpotlightScoresEventId;
    // Song id to the scores per instrument.
    eastl::map<int, eastl::vector<RBSpotlightHighScore>> mSpotlightScores;
    // Saved but never set; the name is weak.
    int mCachedPointsEventId;
    int mCachedSpotlightRivalsPoints;     // "clan_current_spotlight_rivals_points"
    int mCachedXpRivalsPoints;            // "clan_current_xp_rivals_points"
    int mCachedRankPointsRivalsPoints;    // "clan_current_rank_points_rivals_points"
    int mCachedCurrentRivalsPoints;       // "clan_current_rivals_points"
};

static_assert(offsetof(RBClanPersistentData, mProfile) == 24);
static_assert(offsetof(RBClanPersistentData, mSeenEventId) == 32);
static_assert(offsetof(RBClanPersistentData, mCachedTier) == 44);
static_assert(offsetof(RBClanPersistentData, mSpotlightScoresEventId) == 52);
static_assert(offsetof(RBClanPersistentData, mSpotlightScores) == 56);
static_assert(offsetof(RBClanPersistentData, mCachedPointsEventId) == 112);
static_assert(offsetof(RBClanPersistentData, mCachedCurrentRivalsPoints) == 128);
static_assert(sizeof(RBClanPersistentData) == 136);
