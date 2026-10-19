// rb_meta/RBCampaignGigProgressPersistentData.o (0xD4CCA0 to 0xD4D51B). The
// static initializer at 0xD4D4E0 sets the shared header globals every
// rb_meta object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBCampaignGigProgressPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reconstructed from eboot.elf at 0xD4CCA0. Clear is inlined.
RBCampaignGigProgressPersistentData::RBCampaignGigProgressPersistentData(RBProfile* profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD4CD70.
void RBCampaignGigProgressPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mVenueLocation = Symbol("");
    mHaveAttempted = false;
    mHighestScore = 0;
    mStarsEarned = 0;
    mDisplayStars = 0;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4CE40. The copy pushes the heap of the
// constructor and then the assignment's.
RBCampaignGigProgressPersistentData::RBCampaignGigProgressPersistentData(
    const RBCampaignGigProgressPersistentData& other) {
    mParent = other.mParent;
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    *this = other;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4CF40.
RBCampaignGigProgressPersistentData& RBCampaignGigProgressPersistentData::operator=(
    const RBCampaignGigProgressPersistentData& other) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mVenueLocation = other.mVenueLocation;
    mStarsEarned = other.mStarsEarned;
    mDisplayStars = other.mDisplayStars;
    mHighestScore = other.mHighestScore;
    mHaveAttempted = other.mHaveAttempted;
    MakeDirty();
    MemPopHeap();
    return *this;
}

// Reconstructed from eboot.elf at 0xD4CE20. The deleting destructor is at
// 0xD4CE30.
RBCampaignGigProgressPersistentData::~RBCampaignGigProgressPersistentData() {}

// Reconstructed from eboot.elf at 0xD4CFD0.
void RBCampaignGigProgressPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        revisionStream.BeginWrite();
        revisionStream << mVenueLocation;
        int value = mStarsEarned;
        revisionStream.WriteEndian(&value, sizeof(value));
        bool attempted = mHaveAttempted;
        revisionStream.Write(&attempted, sizeof(attempted));
        value = mHighestScore;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mDisplayStars;
        revisionStream.WriteEndian(&value, sizeof(value));
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D0F0.
void RBCampaignGigProgressPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<Symbol>(&mVenueLocation, 0);
            revisionStream >> Revision::AsOfOp<int>(&mStarsEarned, 0);
            revisionStream >> Revision::AsOfOp<bool>(&mHaveAttempted, 0);
            revisionStream >> Revision::AsOfOp<int>(&mHighestScore, 1);
            revisionStream >> Revision::AsOfOp<int>(&mDisplayStars, 2);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D2B0.
void RBCampaignGigProgressPersistentData::SetVenueLocation(const Symbol& venue) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mVenueLocation = venue;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D320.
void RBCampaignGigProgressPersistentData::SetStarsEarned(int stars) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mStarsEarned = stars;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D390.
void RBCampaignGigProgressPersistentData::SetDisplayStars(int stars) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mDisplayStars = stars;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D400.
void RBCampaignGigProgressPersistentData::SetHighestScore(int score) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mHighestScore = score;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D470.
void RBCampaignGigProgressPersistentData::SetHaveAttempted(bool attempted) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mHaveAttempted = attempted;
    MakeDirty();
    MemPopHeap();
}
