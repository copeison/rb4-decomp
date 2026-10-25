#pragma once

#include <cstddef>

#include "persistence/core/PersistentData.h"
#include "utl/containers/Set.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;

namespace RBAch {

struct CampaignGigPerformanceArgs;
struct FinalizedSongStatsArgs;
struct SongPerformanceArgs;

// The achievement progress the platform halves keep over the songs played
// (rb_meta/RBAchUtl.o, 0xF04990-0xF0727F): per-song proficiency lists that
// the fan, master, immortal, gold star, authored solo and vocal trophies
// read. The vtable at 0x198F838 has PersistentData's 7 slots (destructors
// 0xF05F80 and 0xF05F90, Save 0xF04A90, Load 0xF04DD0); the object is 216
// bytes. The destructor is implicit: RBAchPersistentData_PS4.o emits its
// body (0xF04740). Declared only: RBAchUtl.o is not reconstructed.
class SonglistProficiency : public PersistentData {
public:
    explicit SonglistProficiency(IDirty* parent);  // 0xF04990

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xF04A90
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xF04DD0

    void AddPerformance(const FinalizedSongStatsArgs& args);  // 0xF04FB0
    void AddPerformance(const SongPerformanceArgs& args);     // 0xF054D0
    bool IsRockFan() const;                                   // 0xF05600
    bool IsRockMaster() const;                                // 0xF05620
    bool IsRockImmortal() const;                              // 0xF05650
    bool IsGoldStar() const;                                  // 0xF05680
    bool IsAuthoredSoloMaster() const;                        // 0xF056A0
    bool IsVocalMaster() const;                               // 0xF056D0

    // The six song lists. Field names are not in the reference map, and the
    // element type is weak (RBAchUtl.o instantiates
    // Revision::AsOfOp<eastl::vector<int>>).
    eastl::vector<int> mFanSongs;
    eastl::vector<int> mMasterSongs;
    eastl::vector<int> mImmortalSongs;
    eastl::vector<int> mGoldStarSongs;
    eastl::vector<int> mAuthoredSoloSongs;
    eastl::vector<int> mVocalSongs;
};

static_assert(offsetof(SonglistProficiency, mFanSongs) == 24);
static_assert(offsetof(SonglistProficiency, mVocalSongs) == 184);
static_assert(sizeof(SonglistProficiency) == 216);

// The campaign progress the achievements count: the gigs and bonus sets
// attempted and two running totals (rb_meta/RBAchUtl.o). The vtable at
// 0x198F880 has PersistentData's 7 slots (Save 0xF05C50, Load 0xF05DF0);
// the object is 144 bytes. The destructor is implicit:
// RBAchPersistentData_PS4's destructor inlines it. Declared only.
class CampaignData : public PersistentData {
public:
    explicit CampaignData(IDirty* parent);  // 0xF05900

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xF05C50
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xF05DF0

    // Records the gig and the bonus set, and adds the gig's two totals.
    void LogCampaignGigPerformed(const CampaignGigPerformanceArgs& args);  // 0xF05A30
    unsigned long NumUniqueBonusSetsAttempted() const;   // 0xF05A10
    unsigned long NumUniqueCampaignGigsAttempted() const;  // 0xF05A20

    // Field names are not in the reference map; the totals' meaning is weak
    // (the args' +24 and +28 accumulated).
    eastl::set<Symbol> mGigsAttempted;
    eastl::set<Symbol> mBonusSetsAttempted;
    int mFansEarned;
    int mCashEarned;
};

static_assert(offsetof(CampaignData, mGigsAttempted) == 24);
static_assert(offsetof(CampaignData, mBonusSetsAttempted) == 80);
static_assert(offsetof(CampaignData, mFansEarned) == 136);
static_assert(offsetof(CampaignData, mCashEarned) == 140);
static_assert(sizeof(CampaignData) == 144);

}  // namespace RBAch
