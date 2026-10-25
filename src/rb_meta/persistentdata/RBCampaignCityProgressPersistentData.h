#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "rb_meta/persistentdata/RBCampaignGigProgressPersistentData.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// A campaign band's saved progress in one city
// (rb_meta/RBCampaignCityProgressPersistentData.o, 0xD4B660-0xD4CC9F): the
// fans and cash earned there and the progress of each gig, keyed by the
// gig's name. A band keeps one per city it reached. The vtable at 0x197D6F0
// has PersistentData's 7 slots; the object is 96 bytes. The gigs' parent is the city: adding,
// copying and loading them re-points it.
class RBCampaignCityProgressPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    // The revision Save writes. Name not in the reference map.
    static constexpr int kRevision = 0;

    // The band's map builds its nodes with the default, a null profile
    // (0xD4B410).
    explicit RBCampaignCityProgressPersistentData(RBProfile* profile = nullptr);  // 0xD4B660
    // The map's build has no copy constructor or assignment; the binary
    // emits them for the band's map. The copy takes the parent and the
    // profile, the assignment copies the totals and the gigs, re-points the
    // gigs at this city and marks it dirty.
    RBCampaignCityProgressPersistentData(const RBCampaignCityProgressPersistentData& other);  // 0xD4B810
    RBCampaignCityProgressPersistentData& operator=(const RBCampaignCityProgressPersistentData& other);  // 0xD4B920
    // Slots 0-1: 0xD4B7B0, 0xD4B7D0.
    ~RBCampaignCityProgressPersistentData() override;

    // Slot 5: the fans, the cash, the gig count and each gig's name and
    // block.
    void Save(BinStream& stream) const override;  // 0xD4BA60
    // Slot 6: everything as of revision 0. The gigs are added to those
    // present, not replacing them.
    void Load(BinStream& stream) override;  // 0xD4BBC0

    // Forgets the fans, the cash and the gigs without marking the city
    // dirty. The constructor calls it.
    void Clear();  // 0xD4B720

    // The profile is not used.
    void AddFansEarned(RBProfile* profile, int fans);  // 0xD4BD60
    void AddCashEarned(int cash);                      // 0xD4BDD0
    // The sum of the gigs' highest scores. The map gives no return type;
    // the binary sums into 64 bits.
    long GetTotalBandScoreInCity() const;  // 0xD4BE40
    int GetDisplayStarsEarnedInCity() const;  // 0xD4BE80
    // Counts the unattempted gigs whose names are not in `gigs`.
    int GetNumUnattemptedGigs(const eastl::list<Symbol>& gigs) const;  // 0xD4BEC0
    int GetNumAttemptedGigs() const;  // 0xD4BF50
    // Replaces the gig's progress with a fresh one at the venue.
    void AddNewGig(const Symbol& gig, const Symbol& venue);  // 0xD4BF90
    // Null for an unknown gig.
    RBCampaignGigProgressPersistentData* GetGigProgress(const Symbol& gig);  // 0xD4C210
    const RBCampaignGigProgressPersistentData* GetGigProgress(const Symbol& gig) const;  // 0xD4C2A0
    // Appends the names of the gigs at the venue.
    void GetGigsAtVenue(const Symbol& venue, eastl::list<Symbol>& gigs) const;  // 0xD4C330
    bool GetVenueForGig(const Symbol& gig, Symbol& venue) const;  // 0xD4C3C0
    // Appends every gig's name.
    void GetAllAssignedGigs(eastl::list<Symbol>& gigs) const;  // 0xD4C420
    // Keeps the better score and stars, marks the gig attempted and adds
    // the fans and cash. The gig must exist.
    void SetGigCompletionResults(
        RBProfile* profile, const Symbol& gig, int score, int stars, int displayStars, int fans, int cash);  // 0xD4C4A0
    int GetNumberOfGigsAtVenue(const Symbol& venue) const;  // 0xD4C6B0
    bool GetHasGigBeenAttempted(const Symbol& gig) const;  // 0xD4C700
    // Whether any gig was attempted.
    bool HasProgress() const;  // 0xD4C750

    // Field names are not in the reference map; the adders name the totals.
    int mFansEarned;
    int mCashEarned;
    eastl::map<Symbol, RBCampaignGigProgressPersistentData> mGigProgress;
    RBProfile* mProfile;
};

static_assert(offsetof(RBCampaignCityProgressPersistentData, mFansEarned) == 24);
static_assert(offsetof(RBCampaignCityProgressPersistentData, mCashEarned) == 28);
static_assert(offsetof(RBCampaignCityProgressPersistentData, mGigProgress) == 32);
static_assert(offsetof(RBCampaignCityProgressPersistentData, mProfile) == 88);
static_assert(sizeof(RBCampaignCityProgressPersistentData) == 96);
