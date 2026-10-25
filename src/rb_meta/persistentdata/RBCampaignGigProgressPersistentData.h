#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// A campaign gig's saved progress (rb_meta/RBCampaignGigProgressPersistentData.o,
// 0xD4CCA0-0xD4D51B): where it was played, its stars and its best score. A
// city's progress keeps one per gig in a map keyed by the gig's name. The
// vtable at 0x197D760 has PersistentData's 7 slots; the object is 48 bytes.
// Every setter stores the value under the "profile" heap and marks the gig
// dirty, which dirties the profile.
class RBCampaignGigProgressPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    // The revision Save writes. Name not in the reference map.
    static constexpr int kRevision = 2;

    // The city's map builds its nodes with the default, a null profile
    // (0xD4CA50).
    explicit RBCampaignGigProgressPersistentData(RBProfile* profile = nullptr);  // 0xD4CCA0
    // The map's build has no copy constructor or assignment; the binary
    // emits them for the city's map. They copy the values and mark the
    // copy dirty.
    RBCampaignGigProgressPersistentData(const RBCampaignGigProgressPersistentData& other);  // 0xD4CE40
    RBCampaignGigProgressPersistentData& operator=(const RBCampaignGigProgressPersistentData& other);  // 0xD4CF40
    // Slots 0-1: 0xD4CE20 (empty), 0xD4CE30.
    ~RBCampaignGigProgressPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD4CFD0
    // Slot 6: the venue, stars and attempted flag from revision 0, the
    // highest score from 1 and the display stars from 2.
    void Load(BinStream& stream) override;  // 0xD4D0F0

    // Forgets the progress. The constructor inlines it.
    void Clear();  // 0xD4CD70

    void SetVenueLocation(const Symbol& venue);  // 0xD4D2B0
    void SetStarsEarned(int stars);              // 0xD4D320
    void SetDisplayStars(int stars);             // 0xD4D390
    void SetHighestScore(int score);             // 0xD4D400
    void SetHaveAttempted(bool attempted);       // 0xD4D470

    // Inline in the map's build.
    Symbol GetVenueLocation() const {
        return mVenueLocation;
    }
    int GetStarsEarned() const {
        return mStarsEarned;
    }
    int GetDisplayStars() const {
        return mDisplayStars;
    }
    int GetHighestScore() const {
        return mHighestScore;
    }
    bool GetHaveAttempted() const {
        return mHaveAttempted;
    }

    // Field names are not in the reference map; the setters name them.
    Symbol mVenueLocation;
    int mStarsEarned;
    int mDisplayStars;
    int mHighestScore;
    bool mHaveAttempted;
};

static_assert(offsetof(RBCampaignGigProgressPersistentData, mVenueLocation) == 24);
static_assert(offsetof(RBCampaignGigProgressPersistentData, mStarsEarned) == 32);
static_assert(offsetof(RBCampaignGigProgressPersistentData, mDisplayStars) == 36);
static_assert(offsetof(RBCampaignGigProgressPersistentData, mHighestScore) == 40);
static_assert(offsetof(RBCampaignGigProgressPersistentData, mHaveAttempted) == 44);
static_assert(sizeof(RBCampaignGigProgressPersistentData) == 48);
