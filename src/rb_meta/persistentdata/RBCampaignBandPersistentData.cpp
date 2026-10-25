// rb_meta/RBCampaignBandPersistentData.o (0xD464A0 to 0xD4B65F). The
// object also emits the revision reads Load uses, Revision::AsOfOp<T> for
// String (vtable 0x197D518), the city map (0x197D540), the tour list
// (0x197D568), unsigned int (0x197D590), the session musician map
// (0x197D5E0), map<Symbol, int> (0x197D608), the song map (0x197D630),
// set<int> (0x197D658), HxGuid (0x197D680) and the story data (0x197D6A8),
// and RangeOp<map<int, Symbol>> (0x197D5B8): their destructors are at
// 0xD48480-0xD48520 and next to each Read (0xD4A5F0-0xD4B3F0). It emits the
// map readers below (0xD4A640, 0xD4A980, 0xD4AC80, 0xD4ADD0, 0xD4AF20), the
// list's resize (0xD4A820) and the maps' DoNukeSubtree (0xD4A1D0,
// 0xD4A230, 0xD4A280), DoCopySubtree (0xD4A2E0, 0xD4A3D0, 0xD4A4E0) and key
// inserts (0xD4AA90, 0xD4B030, 0xD4B150, 0xD4B410, 0xD4B520). The
// map<Symbol, int> and list<Symbol> helpers it calls are shared copies in
// the game code. The static initializer at 0xD4B620 sets the shared header
// globals every rb_meta object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBCampaignBandPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/persistentdata/RBAchPersistentData.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/songs/SongMetadata.h"
#include "rb_meta/songs/SongMgr.h"
#include "utl/streams/RevisionStream.h"

// ---------------------------------------------------------------------------
// Declared for the band: rb_game's campaign manager (rb_game/RBCampaignMgr.o)
// is not modelled. Only the members SetBandName and Load use are declared.

class RBCampaignMgr {
public:
    // The manager is polymorphic (its vtable is at 0x1CB4190 in the map's
    // build); the destructor is not defined here.
    virtual ~RBCampaignMgr();

    // Reports a band's new name; the flag marks a band named for the first
    // time. The map's signature.
    void SendBandNameBI(Symbol oldName, Symbol newName, bool newBand);  // 0x953EE0
    // The pending fix that restores a fan gig older saves overwrote,
    // "stomped_fan_gig_fix", built on first use. Name not in the reference
    // map; the const qualifier is a guess.
    Symbol GetStompedFanGigFix() const;  // 0x9524E0

    // The active campaign's profile and band (the map's
    // ActivateCampaignForProfile(RBProfile*, RBCampaignBandPersistentData*)
    // takes both; the game reads the band at +16). Field names not in the
    // reference map; the profile's is weak.
    RBProfile* mActiveProfile;
    RBCampaignBandPersistentData* mActiveBand;
};

extern RBCampaignMgr theRBCampaignMgr;  // 0x1AE03C0

// ---------------------------------------------------------------------------
// The readers of the saved containers. The maps' are the map's generic
// operator>> <Key, T> (utl/containers/Map.h); the others are inlined into
// their AsOfOp's Read. Names not in the reference map unless stated.

// Reads a count and that many names into the list (AsOfOp's Read at
// 0xD4A770). The map's operator>> <Symbol, HmxAllocator::allocator>.
static BinStream& operator>>(BinStream& stream, eastl::list<Symbol>& symbols) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    symbols.resize(count, Symbol());
    for (auto symbol = symbols.begin(); symbol != symbols.end(); ++symbol) {
        stream >> *symbol;
    }
    return stream;
}

// Reads a count and that many names into the vector (the shared AsOfOp's
// Read at 0x9D5790).
static BinStream& operator>>(BinStream& stream, eastl::vector<Symbol>& symbols) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    symbols.resize(count);
    for (Symbol* symbol = symbols.begin(); symbol != symbols.end(); ++symbol) {
        stream >> *symbol;
    }
    return stream;
}

// Reads a count and that many ids into the set (AsOfOp's Read at 0xD4B260).
static BinStream& operator>>(BinStream& stream, eastl::set<int>& ids) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (unsigned int i = 0; i < count; ++i) {
        int id;
        stream >> id;
        ids.insert(id);
    }
    return stream;
}

// Reads a count and that many city names and blocks, loading each into the
// city of that name (0xD4A640).
static BinStream& operator>>(BinStream& stream, eastl::map<Symbol, RBCampaignCityProgressPersistentData>& cities) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        Symbol city;
        stream >> city;
        cities[city].Load(stream);
    }
    return stream;
}

// The session musicians of revisions 9-16, characters by part (0xD4A980).
static BinStream& operator>>(BinStream& stream, eastl::map<int, Symbol>& characters) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        int track;
        stream >> track;
        stream >> characters[track];
    }
    return stream;
}

// The session musicians from revision 17: the part, the character and the
// flag (0xD4AC80).
static BinStream& operator>>(BinStream& stream, eastl::map<TrackType, SessionMusician>& musicians) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        int track;
        stream >> track;
        SessionMusician& musician = musicians[static_cast<TrackType>(track)];
        stream >> musician.mCharacter >> musician.mIsDefault;
    }
    return stream;
}

// The tour totals (0xD4ADD0).
static BinStream& operator>>(BinStream& stream, eastl::map<Symbol, int>& totals) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        Symbol tour;
        stream >> tour;
        stream >> totals[tour];
    }
    return stream;
}

// The song map's reader (0xD4AF20) builds its nodes with
// SongPersistentData(nullptr, 0), which SongPersistentData.h does not
// allow as a default yet; it stays the declared generic operator>> <int,
// SongPersistentData> of utl/containers/Map.h.

// Loads the story data (AsOfOp's Read at 0xD4B3F0 calls Load directly).
static BinStream& operator>>(BinStream& stream, RBBTMBandPersistentData& data) {
    data.Load(stream);
    return stream;
}

// ---------------------------------------------------------------------------

// Reconstructed from eboot.elf at 0xD464A0.
RBCampaignBandPersistentData::RBCampaignBandPersistentData(RBProfile* profile)
    : mBTMData(this), mProfile(profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD46820.
void RBCampaignBandPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mBandName = gNullStr;
    mHomeCity = Symbol(gNullStr);
    mLastVisitedCity = Symbol(gNullStr);
    mCurrentTour = Symbol(gNullStr);
    mLastCompletedTourGig = Symbol(gNullStr);
    mCityForNextTourChoice = Symbol(gNullStr);
    mCompletedTours.clear();
    mTourFans.clear();
    mTourCash.clear();
    mCityProgress.clear();
    mSessionMusicians.clear();
    mStarsAtTourStart = 0;
    mCompletedCampaign = false;
    mCashSpent = 0;
    mPendingFanGigUnlocks = 0;
    mGuid.Clear();
    mPendingFixes.clear();
    mBTMData.Clear();
    mBTMData.SetBand(this);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD46B50. The deleting destructor is at
// 0xD46C50.
RBCampaignBandPersistentData::~RBCampaignBandPersistentData() {}

// Reconstructed from eboot.elf at 0xD46C70. The copy pushes the heap of the
// constructor and then the assignment's; it leaves the recently played
// flag unset.
RBCampaignBandPersistentData::RBCampaignBandPersistentData(const RBCampaignBandPersistentData& other)
    : mBTMData(this) {
    mParent = other.mParent;
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mProfile = other.mProfile;
    *this = other;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD47050. The cities are re-pointed also
// on a self-assignment.
RBCampaignBandPersistentData& RBCampaignBandPersistentData::operator=(const RBCampaignBandPersistentData& other) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mBandName = other.mBandName;
    mHomeCity = other.mHomeCity;
    mLastVisitedCity = other.mLastVisitedCity;
    mCurrentTour = other.mCurrentTour;
    mLastCompletedTourGig = other.mLastCompletedTourGig;
    mCityForNextTourChoice = other.mCityForNextTourChoice;
    mCompletedTours = other.mCompletedTours;
    mTourFans = other.mTourFans;
    mTourCash = other.mTourCash;
    mCityProgress = other.mCityProgress;
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        city->second.mParent = this;
    }
    mSessionMusicians = other.mSessionMusicians;
    mStarsAtTourStart = other.mStarsAtTourStart;
    mCompletedCampaign = other.mCompletedCampaign;
    mCashSpent = other.mCashSpent;
    mPendingFanGigUnlocks = other.mPendingFanGigUnlocks;
    mGuid = other.mGuid;
    mPendingFixes = other.mPendingFixes;
    mBTMData = other.mBTMData;
    mBTMData.SetBand(this);
    MakeDirty();
    MemPopHeap();
    return *this;
}

// Reconstructed from eboot.elf at 0xD475D0.
void RBCampaignBandPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        revisionStream.BeginWrite();
        revisionStream << mBandName;
        revisionStream << mHomeCity;
        revisionStream << mCurrentTour;
        int count = static_cast<int>(mCityProgress.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
            city->second.Save(revisionStream << city->first);
        }
        count = static_cast<int>(mCompletedTours.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (auto tour = mCompletedTours.begin(); tour != mCompletedTours.end(); ++tour) {
            revisionStream << *tour;
        }
        revisionStream << mLastCompletedTourGig;
        revisionStream << mLastVisitedCity;
        int value = mStarsAtTourStart;
        revisionStream.WriteEndian(&value, sizeof(value));
        bool flag = mCompletedCampaign;
        revisionStream.Write(&flag, sizeof(flag));
        unsigned int cash = mCashSpent;
        revisionStream.WriteEndian(&cash, sizeof(cash));
        count = static_cast<int>(mSessionMusicians.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (auto musician = mSessionMusicians.begin(); musician != mSessionMusicians.end(); ++musician) {
            int track = musician->first;
            revisionStream.WriteEndian(&track, sizeof(track));
            BinStream& out = revisionStream << musician->second.mCharacter;
            flag = musician->second.mIsDefault;
            out.Write(&flag, sizeof(flag));
        }
        value = mPendingFanGigUnlocks;
        revisionStream.WriteEndian(&value, sizeof(value));
        flag = mRecentlyPlayed;
        revisionStream.Write(&flag, sizeof(flag));
        count = static_cast<int>(mTourFans.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (auto tour = mTourFans.begin(); tour != mTourFans.end(); ++tour) {
            BinStream& out = revisionStream << tour->first;
            value = tour->second;
            out.WriteEndian(&value, sizeof(value));
        }
        count = static_cast<int>(mTourCash.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (auto tour = mTourCash.begin(); tour != mTourCash.end(); ++tour) {
            BinStream& out = revisionStream << tour->first;
            value = tour->second;
            out.WriteEndian(&value, sizeof(value));
        }
        revisionStream << mCityForNextTourChoice;
        count = static_cast<int>(mSongData.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (auto song = mSongData.begin(); song != mSongData.end(); ++song) {
            int id = song->first;
            revisionStream.WriteEndian(&id, sizeof(id));
            song->second.Save(revisionStream);
        }
        count = static_cast<int>(mSongsPlayed.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (const eastl::rbtree_node_base* node = mSongsPlayed.begin(); node != mSongsPlayed.end();
             node = eastl::RBTreeIncrement(node)) {
            int id = static_cast<const eastl::set<int>::node_type*>(node)->mValue;
            revisionStream.WriteEndian(&id, sizeof(id));
        }
        revisionStream << mGuid;
        count = static_cast<int>(mPendingFixes.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        for (const Symbol* fix = mPendingFixes.begin(); fix != mPendingFixes.end(); ++fix) {
            revisionStream << *fix;
        }
        mBTMData.Save(revisionStream);
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD47B60.
void RBCampaignBandPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<String>(&mBandName, 0);
            revisionStream >> Revision::AsOfOp<Symbol>(&mHomeCity, 1);
            revisionStream >> Revision::AsOfOp<Symbol>(&mCurrentTour, 2);
            revisionStream >> Revision::AsOfOp<eastl::map<Symbol, RBCampaignCityProgressPersistentData>>(
                &mCityProgress, 3);
            for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
                city->second.mParent = this;
            }
            revisionStream >> Revision::AsOfOp<eastl::list<Symbol>>(&mCompletedTours, 4);
            revisionStream >> Revision::AsOfOp<Symbol>(&mLastCompletedTourGig, 5);
            revisionStream >> Revision::AsOfOp<Symbol>(&mLastVisitedCity, 5);
            revisionStream >> Revision::AsOfOp<int>(&mStarsAtTourStart, 6);
            revisionStream >> Revision::AsOfOp<bool>(&mCompletedCampaign, 6);
            revisionStream >> Revision::AsOfOp<unsigned int>(&mCashSpent, 7);
            if (revisionStream.mReadRevision < 8) {
                mCashSpent = 0;
            }
            if (revisionStream.mReadRevision <= 16) {
                // Revisions 9-16 kept only the characters; they load as
                // picked by the game.
                eastl::map<int, Symbol> characters;
                revisionStream >> Revision::RangeOp<eastl::map<int, Symbol>>(&characters, 9, 16);
                for (auto character = characters.begin(); character != characters.end(); ++character) {
                    SessionMusician& musician = mSessionMusicians[static_cast<TrackType>(character->first)];
                    musician.mCharacter = character->second;
                    musician.mIsDefault = true;
                }
            }
            revisionStream >> Revision::AsOfOp<eastl::map<TrackType, SessionMusician>>(&mSessionMusicians, 17);
            revisionStream >> Revision::AsOfOp<int>(&mPendingFanGigUnlocks, 10);
            revisionStream >> Revision::AsOfOp<bool>(&mRecentlyPlayed, 11);
            revisionStream >> Revision::AsOfOp<eastl::map<Symbol, int>>(&mTourFans, 12);
            revisionStream >> Revision::AsOfOp<eastl::map<Symbol, int>>(&mTourCash, 12);
            revisionStream >> Revision::AsOfOp<Symbol>(&mCityForNextTourChoice, 13);
            revisionStream >> Revision::AsOfOp<eastl::map<int, SongPersistentData>>(&mSongData, 14);
            if (revisionStream.mReadRevision >= 14 && revisionStream.mReadRevision <= 17) {
                // Revisions 14-17 kept the songs' results; the ids count as
                // played.
                for (auto song = mSongData.begin(); song != mSongData.end(); ++song) {
                    mSongsPlayed.insert(song->first);
                }
            } else {
                revisionStream >> Revision::AsOfOp<eastl::set<int>>(&mSongsPlayed, 18);
            }
            revisionStream >> Revision::AsOfOp<HxGuid>(&mGuid, 15);
            if (revisionStream.mReadRevision <= 14) {
                mGuid.Generate();
            }
            revisionStream >> Revision::AsOfOp<eastl::vector<Symbol>>(&mPendingFixes, 16);
            if (revisionStream.mReadRevision <= 15) {
                mPendingFixes.push_back(theRBCampaignMgr.GetStompedFanGigFix());
            }
            revisionStream >> Revision::AsOfOp<RBBTMBandPersistentData>(&mBTMData, 19);
            mBTMData.SetBand(this);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48530.
void RBCampaignBandPersistentData::GenerateGuid() {
    mGuid.Generate();
}

// Reconstructed from eboot.elf at 0xD48540.
void RBCampaignBandPersistentData::SetBandName(const String& name) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (theRBCampaignMgr.mActiveProfile != nullptr) {
        const bool newBand = mBandName.c_str()[0] == '\0';
        theRBCampaignMgr.SendBandNameBI(Symbol(mBandName.c_str()), Symbol(name.c_str()), newBand);
    }
    mBandName = name;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48690.
void RBCampaignBandPersistentData::SetHomeCity(const Symbol& city) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mHomeCity = city;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48700.
void RBCampaignBandPersistentData::SetLastVisitedCity(const Symbol& city) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mLastVisitedCity = city;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48770. GetAllStarsEarned is inlined.
void RBCampaignBandPersistentData::SetCurrentTour(const Symbol& tour) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCurrentTour = tour;
    if (tour.Str() != gNullStr) {
        mStarsAtTourStart = GetAllStarsEarned();
    }
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48830.
int RBCampaignBandPersistentData::GetAllStarsEarned() const {
    int stars = 0;
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        stars += city->second.GetDisplayStarsEarnedInCity();
    }
    return stars;
}

// Reconstructed from eboot.elf at 0xD48890.
void RBCampaignBandPersistentData::SetLastCompletedTourGig(const Symbol& gig) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mLastCompletedTourGig = gig;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48900.
void RBCampaignBandPersistentData::SetCityForNextTourChoice(const Symbol& city) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCityForNextTourChoice = city;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48970. The list's sort is the shared
// DoSort at 0x331160; SetLastCompletedTourGig is inlined.
void RBCampaignBandPersistentData::SetCompletedTour(const Symbol& tour) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCompletedTours.push_back(tour);
    mCompletedTours.sort();
    mCompletedTours.unique();
    SetCurrentTour(Symbol(gNullStr));
    SetLastCompletedTourGig(Symbol(gNullStr));
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48B70.
Symbol RBCampaignBandPersistentData::GetLastCompletedTour() const {
    if (!mCompletedTours.empty()) {
        return mCompletedTours.back();
    }
    return Symbol(gNullStr);
}

// Reconstructed from eboot.elf at 0xD48BD0.
void RBCampaignBandPersistentData::SetSessionMusician(TrackType track, SessionMusician musician) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSessionMusicians[track] = musician;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48D00.
SessionMusician RBCampaignBandPersistentData::GetSessionMusician(const TrackType& track) const {
    const auto it = mSessionMusicians.find(track);
    if (it != mSessionMusicians.end()) {
        return it->second;
    }
    return SessionMusician{Symbol(gNullStr), true};
}

// Reconstructed from eboot.elf at 0xD48DC0.
bool RBCampaignBandPersistentData::GetHasSessionMusicians() const {
    return mSessionMusicians.size() != 0;
}

// Reconstructed from eboot.elf at 0xD48DD0.
void RBCampaignBandPersistentData::IncrementPendingFanGigUnlocks() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    ++mPendingFanGigUnlocks;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48E40.
void RBCampaignBandPersistentData::DecrementPendingFanGigUnlocks() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (mPendingFanGigUnlocks > 0) {
        --mPendingFanGigUnlocks;
        MakeDirty();
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48EC0. GetCityProgress is inlined; a
// missing city is not checked.
void RBCampaignBandPersistentData::SetGigCompletionResults(RBProfile* profile, const Symbol& city, const Symbol& gig,
                                                           int score, int stars, int displayStars, int fans,
                                                           int cash) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    GetCityProgress(city)->SetGigCompletionResults(profile, gig, score, stars, displayStars, fans, cash);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD48FF0.
RBCampaignCityProgressPersistentData* RBCampaignBandPersistentData::GetCityProgress(const Symbol& city) {
    const auto it = mCityProgress.find(city);
    return it != mCityProgress.end() ? &it->second : nullptr;
}

// Reconstructed from eboot.elf at 0xD49080. The band is not marked dirty.
void RBCampaignBandPersistentData::AddTourCompletionTotals(const Symbol& tour, int fans, int cash) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    int tourFans;
    int tourCash;
    GetTourCompletionTotals(tour, tourFans, tourCash);
    mTourFans[tour] = tourFans + fans;
    mTourCash[tour] = tourCash + cash;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD49240. The binary looks each total up
// twice.
void RBCampaignBandPersistentData::GetTourCompletionTotals(const Symbol& tour, int& fans, int& cash) const {
    const auto tourFans = mTourFans.find(tour);
    fans = tourFans != mTourFans.end() ? tourFans->second : 0;
    const auto tourCash = mTourCash.find(tour);
    cash = tourCash != mTourCash.end() ? tourCash->second : 0;
}

// Reconstructed from eboot.elf at 0xD49400. The binary first runs a lookup
// of the city whose result it drops (an assertion's, probably).
void RBCampaignBandPersistentData::AddNewCityProgress(const Symbol& city) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    RBCampaignCityProgressPersistentData progress(nullptr);
    mCityProgress[city] = progress;
    mCityProgress[city].mParent = this;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD49600.
const RBCampaignCityProgressPersistentData* RBCampaignBandPersistentData::GetCityProgress(const Symbol& city) const {
    const auto it = mCityProgress.find(city);
    return it != mCityProgress.end() ? &it->second : nullptr;
}

// Reconstructed from eboot.elf at 0xD49690.
void RBCampaignBandPersistentData::GetAllAssignedGigs(eastl::list<Symbol>& gigs) const {
    gigs.clear();
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        city->second.GetAllAssignedGigs(gigs);
    }
}

// Reconstructed from eboot.elf at 0xD49720.
bool RBCampaignBandPersistentData::GetCityForGig(const Symbol& gig, Symbol& city) const {
    for (auto it = mCityProgress.begin(); it != mCityProgress.end(); ++it) {
        Symbol venue;
        if (it->second.GetVenueForGig(gig, venue)) {
            city = it->first;
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD497E0.
void RBCampaignBandPersistentData::GetCitiesForGigs(const eastl::list<Symbol>& gigs,
                                                    eastl::list<Symbol>& cities) const {
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        Symbol venue;
        for (auto it = gigs.begin(); it != gigs.end(); ++it) {
            const Symbol gig = *it;
            if (city->second.GetVenueForGig(gig, venue)) {
                cities.push_back(city->first);
                break;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0xD49910.
bool RBCampaignBandPersistentData::GetVenue(const Symbol& city, const Symbol& gig, Symbol& venue) const {
    const auto it = mCityProgress.find(city);
    if (it != mCityProgress.end()) {
        return it->second.GetVenueForGig(gig, venue);
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD499A0.
long RBCampaignBandPersistentData::GetTotalBandScore() const {
    long total = 0;
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        total += city->second.GetTotalBandScoreInCity();
    }
    return total;
}

// Reconstructed from eboot.elf at 0xD49A00.
void RBCampaignBandPersistentData::GetCampaignTotals(int& stars, int& fans, int& cash) const {
    cash = 0;
    fans = 0;
    stars = 0;
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        stars += city->second.GetDisplayStarsEarnedInCity();
        fans += city->second.mFansEarned;
        cash += city->second.mCashEarned;
    }
    cash -= mCashSpent;
}

// Reconstructed from eboot.elf at 0xD49AA0.
int RBCampaignBandPersistentData::GetNumGigsAttempted() const {
    int count = 0;
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        count += city->second.GetNumAttemptedGigs();
    }
    return count;
}

// Reconstructed from eboot.elf at 0xD49B00.
bool RBCampaignBandPersistentData::HasCityProgressData(const Symbol& city) const {
    return mCityProgress.find(city) != mCityProgress.end();
}

// Reconstructed from eboot.elf at 0xD49B80.
bool RBCampaignBandPersistentData::GetHasCityProgress(const Symbol& city) const {
    const auto it = mCityProgress.find(city);
    if (it != mCityProgress.end()) {
        return it->second.HasProgress();
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD49C10.
void RBCampaignBandPersistentData::SetCompletedCampaign() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCompletedCampaign = true;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD49C80. GetCampaignTotals is inlined;
// its star sums stay as dead calls.
int RBCampaignBandPersistentData::GetAvailableCash() const {
    int stars;
    int fans;
    int cash;
    GetCampaignTotals(stars, fans, cash);
    return cash < 0 ? 0 : cash;
}

// Reconstructed from eboot.elf at 0xD49CF0.
int RBCampaignBandPersistentData::GetTotalCashEarned() const {
    int cash = 0;
    for (auto city = mCityProgress.begin(); city != mCityProgress.end(); ++city) {
        cash += city->second.mCashEarned;
    }
    return cash < 0 ? 0 : cash;
}

// Reconstructed from eboot.elf at 0xD49D30. GetAvailableCash is inlined.
// The profile is the parent's, not mProfile.
bool RBCampaignBandPersistentData::SpendCash(unsigned int cash) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool spent = false;
    if (static_cast<unsigned int>(GetAvailableCash()) >= cash) {
        mCashSpent += cash;
        MakeDirty();
        static_cast<RBProfile*>(mParent)->mAchData->LogCashSpent(static_cast<int>(cash));
        spent = true;
    }
    MemPopHeap();
    return spent;
}

// Reconstructed from eboot.elf at 0xD49E30.
void RBCampaignBandPersistentData::MarkAsRecentlyPlayed(bool recentlyPlayed) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mRecentlyPlayed = recentlyPlayed;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD49EA0. A song already played still
// marks the band dirty.
void RBCampaignBandPersistentData::AddSongPlayed(const int& songID) {
    if (songID >= -2 && songID <= 0) {
        return;
    }
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSongsPlayed.insert(songID);
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD49FD0.
void RBCampaignBandPersistentData::AddSongPlayed(Symbol shortName) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    const int songID = theSongMgr->GetSongIDFromShortName(shortName, false);
    AddSongPlayed(songID);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4A080.
int RBCampaignBandPersistentData::GetDiscSongsPlayed() const {
    int count = 0;
    for (const eastl::rbtree_node_base* node = mSongsPlayed.begin(); node != mSongsPlayed.end();
         node = eastl::RBTreeIncrement(node)) {
        const int songID = static_cast<const eastl::set<int>::node_type*>(node)->mValue;
        const SongMetadata* const metadata = theSongMgr->GetMetadata(songID, false);
        if (metadata != nullptr && metadata->IsRB4Song()) {
            count += !metadata->IsPrivate();
        }
    }
    return count;
}

// Reconstructed from eboot.elf at 0xD4A110. The band is not marked dirty.
void RBCampaignBandPersistentData::RemovePendingFix(const Symbol& fix) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    Symbol* it = mPendingFixes.begin();
    while (it != mPendingFixes.end() && *it != fix) {
        ++it;
    }
    if (it != mPendingFixes.end()) {
        mPendingFixes.erase(it);
    }
    MemPopHeap();
}
