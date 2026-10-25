#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "os/users/HxGuid.h"
#include "persistence/core/PersistentData.h"
#include "rb_beatmatch/tracks/TrackType.h"
#include "rb_meta/persistentdata/RBBTMBandPersistentData.h"
#include "rb_meta/persistentdata/RBCampaignCityProgressPersistentData.h"
#include "rb_meta/persistentdata/SongPersistentData.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/containers/Set.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// A band's session musician for one part: the character standing in and
// whether the game picked it. Older saves only stored the character, which
// loads as picked. Name and field names not in the reference map; the
// evidence for the flag's meaning (RBCampaignMgr::_PickDefaultStandInsForBand
// in the map) is weak.
struct SessionMusician {
    Symbol mCharacter;
    bool mIsDefault;
};

static_assert(sizeof(SessionMusician) == 16);

// A campaign band's saved progress (rb_meta/RBCampaignBandPersistentData.o,
// 0xD464A0-0xD4B65F): its name and guid, home city, current and completed
// tours, the progress in each city, the tour totals, its session
// musicians, cash, the songs it played and its story-mode progress. The
// profile keeps its bands in a vector (RBProfile::mCampaignBands). The
// vtable at 0x197D4F0 has PersistentData's 7 slots; the object is 768
// bytes. The cities' parent is the band: adding, copying and loading them
// re-points it.
class RBCampaignBandPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    // The revision Save writes. Name not in the reference map.
    static constexpr int kRevision = 19;

    explicit RBCampaignBandPersistentData(RBProfile* profile);  // 0xD464A0
    // Not in the map's build; the profile's band vector copies bands with
    // them. The copy takes the parent and the profile; the assignment
    // copies everything except the old song map, the songs played and the
    // recently played flag, re-points the cities and the story data at this
    // band and marks it dirty.
    RBCampaignBandPersistentData(const RBCampaignBandPersistentData& other);  // 0xD46C70
    RBCampaignBandPersistentData& operator=(const RBCampaignBandPersistentData& other);  // 0xD47050
    // Slots 0-1: 0xD46B50, 0xD46C50.
    ~RBCampaignBandPersistentData() override;

    // Slot 5: a revision-19 block.
    void Save(BinStream& stream) const override;  // 0xD475D0
    // Slot 6: each value from the revision that added it. Older saves'
    // session musicians (revisions 9-16) and songs (14-17) are converted,
    // and saves before revision 15 get a new guid.
    void Load(BinStream& stream) override;  // 0xD47B60

    // Resets everything except the songs and the recently played flag,
    // without marking the band dirty. The constructor calls it.
    void Clear();  // 0xD46820

    // Gives a new band its guid.
    void GenerateGuid();  // 0xD48530
    // Reports the rename to the campaign manager when a campaign is active
    // and stores the name.
    void SetBandName(const String& name);  // 0xD48540
    void SetHomeCity(const Symbol& city);  // 0xD48690
    void SetLastVisitedCity(const Symbol& city);  // 0xD48700
    // A tour that starts records the stars earned so far.
    void SetCurrentTour(const Symbol& tour);  // 0xD48770
    int GetAllStarsEarned() const;  // 0xD48830
    void SetLastCompletedTourGig(const Symbol& gig);  // 0xD48890
    void SetCityForNextTourChoice(const Symbol& city);  // 0xD48900
    // Adds the tour to the sorted completed tours and ends the current tour.
    void SetCompletedTour(const Symbol& tour);  // 0xD48970
    // The last completed tour in symbol order, or the empty symbol.
    Symbol GetLastCompletedTour() const;  // 0xD48B70
    // The map has SetSessionMusician(TrackType const&, Symbol const&); this
    // build passes the part and the musician by value.
    void SetSessionMusician(TrackType track, SessionMusician musician);  // 0xD48BD0
    // The part's musician, or the empty symbol picked by the game.
    SessionMusician GetSessionMusician(const TrackType& track) const;  // 0xD48D00
    bool GetHasSessionMusicians() const;  // 0xD48DC0
    void IncrementPendingFanGigUnlocks();  // 0xD48DD0
    // Never below zero.
    void DecrementPendingFanGigUnlocks();  // 0xD48E40
    // Passes the results to the city. The city must exist.
    void SetGigCompletionResults(RBProfile* profile, const Symbol& city, const Symbol& gig, int score, int stars,
                                 int displayStars, int fans, int cash);  // 0xD48EC0
    // Null for an unknown city.
    RBCampaignCityProgressPersistentData* GetCityProgress(const Symbol& city);  // 0xD48FF0
    // Adds to the tour's fan and cash totals.
    void AddTourCompletionTotals(const Symbol& tour, int fans, int cash);  // 0xD49080
    // Zero for a tour without totals.
    void GetTourCompletionTotals(const Symbol& tour, int& fans, int& cash) const;  // 0xD49240
    // Replaces the city's progress with a fresh one.
    void AddNewCityProgress(const Symbol& city);  // 0xD49400
    const RBCampaignCityProgressPersistentData* GetCityProgress(const Symbol& city) const;  // 0xD49600
    // Replaces the list with every city's gigs.
    void GetAllAssignedGigs(eastl::list<Symbol>& gigs) const;  // 0xD49690
    bool GetCityForGig(const Symbol& gig, Symbol& city) const;  // 0xD49720
    // Appends each city that has one of the gigs.
    void GetCitiesForGigs(const eastl::list<Symbol>& gigs, eastl::list<Symbol>& cities) const;  // 0xD497E0
    bool GetVenue(const Symbol& city, const Symbol& gig, Symbol& venue) const;  // 0xD49910
    // The sum of the cities' scores. The map gives no return type; the
    // binary sums into 64 bits.
    long GetTotalBandScore() const;  // 0xD499A0
    // The display stars, the fans and the cash left after spending.
    void GetCampaignTotals(int& stars, int& fans, int& cash) const;  // 0xD49A00
    int GetNumGigsAttempted() const;  // 0xD49AA0
    // Whether the band has the city's progress. Name not in the reference
    // map; the evidence that the map's GetHasCityProgress is 0xD49B80 is
    // weak.
    bool HasCityProgressData(const Symbol& city) const;  // 0xD49B00
    // Whether the city's progress has an attempted gig.
    bool GetHasCityProgress(const Symbol& city) const;  // 0xD49B80
    void SetCompletedCampaign();  // 0xD49C10
    // The cash earned less the cash spent, never below zero.
    int GetAvailableCash() const;  // 0xD49C80
    // Never below zero.
    int GetTotalCashEarned() const;  // 0xD49CF0
    // Spends the cash when the band has enough, marks it dirty and logs the
    // spending with the parent profile's achievements.
    bool SpendCash(unsigned int cash);  // 0xD49D30
    void MarkAsRecentlyPlayed(bool recentlyPlayed);  // 0xD49E30
    // Records a played song by id, ignoring ids -2 to 0. Name not in the
    // reference map.
    void AddSongPlayed(const int& songID);  // 0xD49EA0
    // Records a played song by short name. Name not in the reference map.
    void AddSongPlayed(Symbol shortName);  // 0xD49FD0
    // The played songs that are public Rock Band 4 songs.
    int GetDiscSongsPlayed() const;  // 0xD4A080
    // Drops the first pending fix of that name. Name not in the reference
    // map.
    void RemovePendingFix(const Symbol& fix);  // 0xD4A110

    // Field names are not in the reference map; the setters name most of
    // them. The revision that added each to the save follows it.
    String mBandName;                   // 0
    Symbol mHomeCity;                   // 1
    Symbol mLastVisitedCity;            // 5
    Symbol mCurrentTour;                // 2
    Symbol mLastCompletedTourGig;       // 5
    Symbol mCityForNextTourChoice;      // 13
    eastl::list<Symbol> mCompletedTours;  // 4, kept sorted
    eastl::map<Symbol, int> mTourFans;    // 12
    eastl::map<Symbol, int> mTourCash;    // 12
    eastl::map<Symbol, RBCampaignCityProgressPersistentData> mCityProgress;  // 3
    // Revision 17; 9-16 stored only the characters.
    eastl::map<TrackType, SessionMusician> mSessionMusicians;
    // The stars earned when the current tour started (6).
    int mStarsAtTourStart;
    bool mCompletedCampaign;            // 6
    unsigned int mCashSpent;            // 7
    int mPendingFanGigUnlocks;          // 10
    // Set on the band the profile played last (11); neither the
    // constructor nor Clear sets it. MarkAsRecentlyPlayed sets it.
    bool mRecentlyPlayed;
    HxGuid mGuid;                       // 15
    // The songs' results of revisions 14-17, converted to mSongsPlayed
    // when loaded; kept and saved, but no longer filled.
    eastl::map<int, SongPersistentData> mSongData;
    // The ids of the songs the band played (18).
    eastl::set<int> mSongsPlayed;
    // One-shot fixes the campaign manager applies to the band (16); saves
    // before revision 16 get the "stomped_fan_gig_fix".
    eastl::vector<Symbol> mPendingFixes;
    RBBTMBandPersistentData mBTMData;   // 19
    RBProfile* mProfile;
};

static_assert(offsetof(RBCampaignBandPersistentData, mBandName) == 24);
static_assert(offsetof(RBCampaignBandPersistentData, mHomeCity) == 40);
static_assert(offsetof(RBCampaignBandPersistentData, mCityForNextTourChoice) == 72);
static_assert(offsetof(RBCampaignBandPersistentData, mCompletedTours) == 80);
static_assert(offsetof(RBCampaignBandPersistentData, mTourFans) == 112);
static_assert(offsetof(RBCampaignBandPersistentData, mTourCash) == 168);
static_assert(offsetof(RBCampaignBandPersistentData, mCityProgress) == 224);
static_assert(offsetof(RBCampaignBandPersistentData, mSessionMusicians) == 280);
static_assert(offsetof(RBCampaignBandPersistentData, mStarsAtTourStart) == 336);
static_assert(offsetof(RBCampaignBandPersistentData, mCompletedCampaign) == 340);
static_assert(offsetof(RBCampaignBandPersistentData, mCashSpent) == 344);
static_assert(offsetof(RBCampaignBandPersistentData, mPendingFanGigUnlocks) == 348);
static_assert(offsetof(RBCampaignBandPersistentData, mRecentlyPlayed) == 352);
static_assert(offsetof(RBCampaignBandPersistentData, mGuid) == 356);
static_assert(offsetof(RBCampaignBandPersistentData, mSongData) == 376);
static_assert(offsetof(RBCampaignBandPersistentData, mSongsPlayed) == 432);
static_assert(offsetof(RBCampaignBandPersistentData, mPendingFixes) == 488);
static_assert(offsetof(RBCampaignBandPersistentData, mBTMData) == 520);
static_assert(offsetof(RBCampaignBandPersistentData, mProfile) == 760);
static_assert(sizeof(RBCampaignBandPersistentData) == 768);
