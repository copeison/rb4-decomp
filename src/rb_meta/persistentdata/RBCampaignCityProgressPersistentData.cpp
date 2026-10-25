// rb_meta/RBCampaignCityProgressPersistentData.o (0xD4B660 to 0xD4CC9F). The
// object also emits Revision::AsOfOp<eastl::map<Symbol,
// RBCampaignGigProgressPersistentData>> (vtable 0x197D718: destructors
// 0xD4BD50 and 0xD4C900, Read 0xD4C910), the map reader below (0xD4C930)
// and the gig map's templates: DoNukeSubtree (0xD4C790), DoCopySubtree
// (0xD4C7F0) and the hinted and plain key inserts (0xD4CA50, 0xD4CB60). The
// static initializer at 0xD4CC60 sets the shared header globals every
// rb_meta object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBCampaignCityProgressPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reads a count and that many gig names and blocks, loading each into the
// gig of that name, as the map's operator>> <Symbol,
// RBCampaignGigProgressPersistentData> does (0xD4C930). Existing gigs are
// kept. The generic reader is declared in utl/containers/Map.h.
static BinStream& operator>>(BinStream& stream, eastl::map<Symbol, RBCampaignGigProgressPersistentData>& gigs) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        Symbol gig;
        stream >> gig;
        gigs[gig].Load(stream);
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xD4B660.
RBCampaignCityProgressPersistentData::RBCampaignCityProgressPersistentData(RBProfile* profile)
    : mProfile(profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD4B720.
void RBCampaignCityProgressPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mFansEarned = 0;
    mCashEarned = 0;
    mGigProgress.clear();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4B7B0. The deleting destructor is at
// 0xD4B7D0.
RBCampaignCityProgressPersistentData::~RBCampaignCityProgressPersistentData() {}

// Reconstructed from eboot.elf at 0xD4B810. The copy pushes the heap of the
// constructor and then the assignment's.
RBCampaignCityProgressPersistentData::RBCampaignCityProgressPersistentData(
    const RBCampaignCityProgressPersistentData& other) {
    mParent = other.mParent;
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mProfile = other.mProfile;
    *this = other;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4B920. The gigs are re-pointed also on
// a self-assignment.
RBCampaignCityProgressPersistentData& RBCampaignCityProgressPersistentData::operator=(
    const RBCampaignCityProgressPersistentData& other) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mFansEarned = other.mFansEarned;
    mCashEarned = other.mCashEarned;
    mGigProgress = other.mGigProgress;
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        gig->second.mParent = this;
    }
    MakeDirty();
    MemPopHeap();
    return *this;
}

// Reconstructed from eboot.elf at 0xD4BA60.
void RBCampaignCityProgressPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        revisionStream.BeginWrite();
        int value = mFansEarned;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCashEarned;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = static_cast<int>(mGigProgress.size());
        revisionStream.WriteEndian(&value, sizeof(value));
        for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
            gig->second.Save(revisionStream << gig->first);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4BBC0.
void RBCampaignCityProgressPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<int>(&mFansEarned, 0);
            revisionStream >> Revision::AsOfOp<int>(&mCashEarned, 0);
            revisionStream >> Revision::AsOfOp<eastl::map<Symbol, RBCampaignGigProgressPersistentData>>(
                &mGigProgress, 0);
            for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
                gig->second.mParent = this;
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4BD60.
void RBCampaignCityProgressPersistentData::AddFansEarned(RBProfile* profile, int fans) {
    static_cast<void>(profile);
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mFansEarned += fans;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4BDD0.
void RBCampaignCityProgressPersistentData::AddCashEarned(int cash) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCashEarned += cash;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4BE40.
long RBCampaignCityProgressPersistentData::GetTotalBandScoreInCity() const {
    long total = 0;
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        total += gig->second.GetHighestScore();
    }
    return total;
}

// Reconstructed from eboot.elf at 0xD4BE80.
int RBCampaignCityProgressPersistentData::GetDisplayStarsEarnedInCity() const {
    int stars = 0;
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        stars += gig->second.GetDisplayStars();
    }
    return stars;
}

// Reconstructed from eboot.elf at 0xD4BEC0.
int RBCampaignCityProgressPersistentData::GetNumUnattemptedGigs(const eastl::list<Symbol>& gigs) const {
    int count = 0;
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        auto name = gigs.begin();
        while (name != gigs.end() && *name != gig->first) {
            ++name;
        }
        if (name == gigs.end() && !gig->second.GetHaveAttempted()) {
            ++count;
        }
    }
    return count;
}

// Reconstructed from eboot.elf at 0xD4BF50.
int RBCampaignCityProgressPersistentData::GetNumAttemptedGigs() const {
    int count = 0;
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        count += gig->second.GetHaveAttempted();
    }
    return count;
}

// Reconstructed from eboot.elf at 0xD4BF90. The binary first runs a lookup
// of the gig whose result it drops (an assertion's, probably).
void RBCampaignCityProgressPersistentData::AddNewGig(const Symbol& gig, const Symbol& venue) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mGigProgress[gig] = RBCampaignGigProgressPersistentData(nullptr);
    mGigProgress[gig].mParent = this;
    mGigProgress[gig].SetVenueLocation(venue);
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4C210.
RBCampaignGigProgressPersistentData* RBCampaignCityProgressPersistentData::GetGigProgress(const Symbol& gig) {
    const auto it = mGigProgress.find(gig);
    return it != mGigProgress.end() ? &it->second : nullptr;
}

// Reconstructed from eboot.elf at 0xD4C2A0.
const RBCampaignGigProgressPersistentData* RBCampaignCityProgressPersistentData::GetGigProgress(
    const Symbol& gig) const {
    const auto it = mGigProgress.find(gig);
    return it != mGigProgress.end() ? &it->second : nullptr;
}

// Reconstructed from eboot.elf at 0xD4C330.
void RBCampaignCityProgressPersistentData::GetGigsAtVenue(const Symbol& venue, eastl::list<Symbol>& gigs) const {
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        if (gig->second.mVenueLocation == venue) {
            gigs.push_back(gig->first);
        }
    }
}

// Reconstructed from eboot.elf at 0xD4C3C0. The gigs are searched in order,
// not looked up.
bool RBCampaignCityProgressPersistentData::GetVenueForGig(const Symbol& gig, Symbol& venue) const {
    for (auto it = mGigProgress.begin(); it != mGigProgress.end(); ++it) {
        if (it->first == gig) {
            venue = it->second.mVenueLocation;
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD4C420.
void RBCampaignCityProgressPersistentData::GetAllAssignedGigs(eastl::list<Symbol>& gigs) const {
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        gigs.push_back(gig->first);
    }
}

// Reconstructed from eboot.elf at 0xD4C4A0. AddFansEarned and AddCashEarned
// are inlined with their heap pushes.
void RBCampaignCityProgressPersistentData::SetGigCompletionResults(
    RBProfile* profile, const Symbol& gig, int score, int stars, int displayStars, int fans, int cash) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    RBCampaignGigProgressPersistentData* const progress = GetGigProgress(gig);
    if (progress->GetHighestScore() < score) {
        progress->SetHighestScore(score);
    }
    if (progress->GetStarsEarned() < stars) {
        progress->SetStarsEarned(stars);
    }
    if (progress->GetDisplayStars() < displayStars) {
        progress->SetDisplayStars(displayStars);
    }
    progress->SetHaveAttempted(true);
    AddFansEarned(profile, fans);
    AddCashEarned(cash);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4C6B0.
int RBCampaignCityProgressPersistentData::GetNumberOfGigsAtVenue(const Symbol& venue) const {
    int count = 0;
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        count += gig->second.mVenueLocation == venue;
    }
    return count;
}

// Reconstructed from eboot.elf at 0xD4C700. The gigs are searched in order.
bool RBCampaignCityProgressPersistentData::GetHasGigBeenAttempted(const Symbol& gig) const {
    for (auto it = mGigProgress.begin(); it != mGigProgress.end(); ++it) {
        if (it->first == gig) {
            return it->second.GetHaveAttempted();
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD4C750.
bool RBCampaignCityProgressPersistentData::HasProgress() const {
    for (auto gig = mGigProgress.begin(); gig != mGigProgress.end(); ++gig) {
        if (gig->second.GetHaveAttempted()) {
            return true;
        }
    }
    return false;
}
