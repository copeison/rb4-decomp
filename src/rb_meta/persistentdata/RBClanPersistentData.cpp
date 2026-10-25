// rb_meta/RBClanPersistentData.o (0xD4FC20 to 0xD52AEB), newer than the map.
// Besides the class the object holds the inline members of the spotlight
// score classes (0xD50C20, 0xD514A0-0xD517FF) and the EASTL instances for
// the three score maps: the vectors' resize and insert, the maps' key
// insertion and subtree deletion (0xD51840-0xD52AAB). The current map's
// subtree deletion is shared with other code (0xBA57F0). The static
// initializer at 0xD52AB0 sets the shared header globals every rb_meta
// object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBClanPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Writes the count, then each key, its vector's size and the elements. The
// generic declaration; the clan data's instance follows.
template <typename Key, typename T>
BinStream& operator<<(BinStream& stream, const eastl::map<Key, T>& map);

// Reconstructed from eboot.elf at 0xD4FFC0.
template <>
BinStream& operator<<(BinStream& stream,
                      const eastl::map<int, eastl::vector<RBSpotlightHighScore>>& map) {
    int count = static_cast<int>(map.mnSize);
    stream.WriteEndian(&count, sizeof(count));
    for (auto it = map.begin(); it != map.end(); ++it) {
        int key = it->first;
        stream.WriteEndian(&key, sizeof(key));
        int size = static_cast<int>(it->second.size());
        stream.WriteEndian(&size, sizeof(size));
        for (const RBSpotlightHighScore* score = it->second.begin(); score != it->second.end();
             ++score) {
            stream << *score;
        }
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xD50A70. Revisions 1-4.
template <>
BinStream& operator>>(BinStream& stream,
                      eastl::map<int, eastl::vector<RBSpotlightHighScoreV1>>& map) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        int key;
        stream.ReadEndian(&key, sizeof(key));
        eastl::vector<RBSpotlightHighScoreV1>& scores = map[key];
        unsigned int size;
        stream.ReadEndian(&size, sizeof(size));
        scores.resize(size);
        for (RBSpotlightHighScoreV1* score = scores.begin(); score != scores.end(); ++score) {
            stream >> *score;
        }
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xD50C30. Revision 5.
template <>
BinStream& operator>>(BinStream& stream,
                      eastl::map<int, eastl::vector<RBSpotlightHighScoreV2>>& map) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        int key;
        stream.ReadEndian(&key, sizeof(key));
        eastl::vector<RBSpotlightHighScoreV2>& scores = map[key];
        unsigned int size;
        stream.ReadEndian(&size, sizeof(size));
        scores.resize(size);
        for (RBSpotlightHighScoreV2* score = scores.begin(); score != scores.end(); ++score) {
            stream >> *score;
        }
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xD50E00. Revision 6 on.
template <>
BinStream& operator>>(BinStream& stream,
                      eastl::map<int, eastl::vector<RBSpotlightHighScore>>& map) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        int key;
        stream.ReadEndian(&key, sizeof(key));
        eastl::vector<RBSpotlightHighScore>& scores = map[key];
        unsigned int size;
        stream.ReadEndian(&size, sizeof(size));
        scores.resize(size);
        for (RBSpotlightHighScore* score = scores.begin(); score != scores.end(); ++score) {
            stream >> *score;
        }
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xD4FC20. Clear is not inlined.
RBClanPersistentData::RBClanPersistentData(RBProfile* profile) : mProfile(profile) {
    mParent = profile;
    Clear();
}

// Reconstructed from eboot.elf at 0xD4FD80. The deleting destructor is at
// 0xD4FDA0.
RBClanPersistentData::~RBClanPersistentData() {}

// Reconstructed from eboot.elf at 0xD4FCD0.
void RBClanPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSeenEventId = -1;
    mCachedLifetimeRivalsPoints = 0;
    mCachedSeasonalRivalsPoints = 0;
    mCachedTier = 6;
    mBestTier = 6;
    mSpotlightScoresEventId = -1;
    mSpotlightScores.clear();
    mCachedPointsEventId = -1;
    mCachedSpotlightRivalsPoints = 0;
    mCachedXpRivalsPoints = 0;
    mCachedRankPointsRivalsPoints = 0;
    mCachedCurrentRivalsPoints = 0;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4FDE0.
const char* RBClanPersistentData::_GetProfileName() const {
    if (mProfile != nullptr) {
        return mProfile->GetPlatformName();
    }
    return "NULL_PROFILE_POINTER!!!";
}

// Reconstructed from eboot.elf at 0xD4FE00.
void RBClanPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        revisionStream.BeginWrite();
        int value = mSeenEventId;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCachedLifetimeRivalsPoints;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCachedSeasonalRivalsPoints;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCachedTier;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mBestTier;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mSpotlightScoresEventId;
        revisionStream.WriteEndian(&value, sizeof(value));
        revisionStream << mSpotlightScores;
        value = mCachedPointsEventId;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCachedSpotlightRivalsPoints;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCachedXpRivalsPoints;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCachedRankPointsRivalsPoints;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mCachedCurrentRivalsPoints;
        revisionStream.WriteEndian(&value, sizeof(value));
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD500B0. The two tiers are read through
// temporaries that are stored even when the block predates them, so an old
// block leaves them undefined, as in the binary. Revision-0 blocks keep the
// scores; the older score layouts are copied into current scores with no
// percentile and not marked as the player's. A block that cannot be read is
// logged with the profile's name.
void RBClanPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, kRevision, 0);
        if (revisionStream.BeginRead()) {
            revisionStream.ReadEndian(&mSeenEventId, sizeof(mSeenEventId));
            revisionStream >> Revision::AsOfOp<int>(&mCachedLifetimeRivalsPoints, 1);
            revisionStream >> Revision::AsOfOp<int>(&mCachedSeasonalRivalsPoints, 2);
            int cachedTier;
            revisionStream >> Revision::AsOfOp<int>(&cachedTier, 1);
            mCachedTier = cachedTier;
            int bestTier;
            revisionStream >> Revision::AsOfOp<int>(&bestTier, 3);
            mBestTier = bestTier;
            revisionStream >> Revision::AsOfOp<int>(&mSpotlightScoresEventId, 1);

            const int revision = revisionStream.mReadRevision;
            if (revision > 0 && revision <= 4) {
                eastl::map<int, eastl::vector<RBSpotlightHighScoreV1>> oldScores;
                revisionStream >> oldScores;
                for (auto it = oldScores.begin(); it != oldScores.end(); ++it) {
                    eastl::vector<RBSpotlightHighScore>& scores = mSpotlightScores[it->first];
                    for (RBSpotlightHighScoreV1* old = it->second.begin(); old != it->second.end();
                         ++old) {
                        RBSpotlightHighScore score;
                        static_cast<RBSpotlightHighScoreV1&>(score) = *old;
                        scores.push_back(score);
                    }
                }
            } else if (revision == 5) {
                eastl::map<int, eastl::vector<RBSpotlightHighScoreV2>> oldScores;
                revisionStream >> oldScores;
                for (auto it = oldScores.begin(); it != oldScores.end(); ++it) {
                    eastl::vector<RBSpotlightHighScore>& scores = mSpotlightScores[it->first];
                    for (RBSpotlightHighScoreV2* old = it->second.begin(); old != it->second.end();
                         ++old) {
                        RBSpotlightHighScore score;
                        static_cast<RBSpotlightHighScoreV2&>(score) = *old;
                        scores.push_back(score);
                    }
                }
            } else if (revision > 5) {
                revisionStream >> mSpotlightScores;
            }

            revisionStream >> Revision::AsOfOp<int>(&mCachedPointsEventId, 1);
            revisionStream >> Revision::AsOfOp<int>(&mCachedSpotlightRivalsPoints, 1);
            revisionStream >> Revision::AsOfOp<int>(&mCachedXpRivalsPoints, 1);
            revisionStream >> Revision::AsOfOp<int>(&mCachedRankPointsRivalsPoints, 4);
            revisionStream >> Revision::AsOfOp<int>(&mCachedCurrentRivalsPoints, 1);
        } else {
            static_cast<void>(_GetProfileName());
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD50FD0.
void RBClanPersistentData::SetSeenEventId(int id) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSeenEventId = id;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD51040.
void RBClanPersistentData::SetCachedLifetimeRivalsPoints(int points) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedLifetimeRivalsPoints = points;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD510B0.
void RBClanPersistentData::SetCachedSeasonalRivalsPoints(int points) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedSeasonalRivalsPoints = points;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD51120.
void RBClanPersistentData::SetCachedTier(int tier) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedTier = tier;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD51190.
void RBClanPersistentData::SetBestTier(int tier) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mBestTier = tier;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD51200.
void RBClanPersistentData::SetSpotlightScoresEventId(int id) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSpotlightScoresEventId = id;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD51270.
void RBClanPersistentData::SetCachedPointsEventId(int id) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedPointsEventId = id;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD512E0.
void RBClanPersistentData::SetCachedSpotlightRivalsPoints(int points) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedSpotlightRivalsPoints = points;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD51350.
void RBClanPersistentData::SetCachedXpRivalsPoints(int points) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedXpRivalsPoints = points;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD513C0.
void RBClanPersistentData::SetCachedRankPointsRivalsPoints(int points) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedRankPointsRivalsPoints = points;
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD51430.
void RBClanPersistentData::SetCachedCurrentRivalsPoints(int points) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCachedCurrentRivalsPoints = points;
    MakeDirty();
    MemPopHeap();
}
