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
// has PersistentData's 7 slots; the object is 96 bytes. Declared only: the
// object is not reconstructed.
class RBCampaignCityProgressPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBCampaignCityProgressPersistentData(RBProfile* profile);  // 0xD4B660
    // Slots 0-1: 0xD4B7B0, 0xD4B7D0.
    ~RBCampaignCityProgressPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD4BA60
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD4BBC0

    // Forgets the fans, the cash and the gigs. The constructor calls it.
    void Clear();  // 0xD4B720

    // The map's signatures; the binary's addresses are not identified.
    void AddFansEarned(RBProfile* profile, int fans);
    void AddCashEarned(int cash);
    int GetTotalBandScoreInCity() const;
    int GetDisplayStarsEarnedInCity() const;
    int GetNumUnattemptedGigs(const eastl::list<Symbol>& gigs) const;
    int GetNumAttemptedGigs() const;
    void AddNewGig(const Symbol& gig, const Symbol& venue);
    RBCampaignGigProgressPersistentData* GetGigProgress(const Symbol& gig);
    const RBCampaignGigProgressPersistentData* GetGigProgress(const Symbol& gig) const;
    void GetGigsAtVenue(const Symbol& venue, eastl::list<Symbol>& gigs) const;
    bool GetVenueForGig(const Symbol& gig, Symbol& venue) const;
    void GetAllAssignedGigs(eastl::list<Symbol>& gigs) const;
    void SetGigCompletionResults(RBProfile* profile, const Symbol& gig, int score, int stars, int displayStars, int fans, int cash);
    int GetNumberOfGigsAtVenue(const Symbol& venue) const;
    bool GetHasGigBeenAttempted(const Symbol& gig) const;
    bool HasProgress() const;

    // Field names are not in the reference map.
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
