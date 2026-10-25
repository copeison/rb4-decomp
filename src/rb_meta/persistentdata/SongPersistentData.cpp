// rb_meta/SongPersistentData.o (0xD69800 to 0xD6DD6F). The std::function
// wrappers of the lambdas below fill 0xD6C850-0xD6DD1F (vtables
// 0x197E368-0x197E918). The static initializer at 0xD6DD20 sets the shared
// header globals every rb_meta object initializes and the "igs_tutorial_event"
// symbol (0x1B20138) of a shared header; it is not modelled. The map's
// SetScore, SetStars, SetAccuracy, GetHighSoloScore, IsUploadDirty and
// ClearUploadDirty are not in this build, and the map's eastl::rbtree
// instances for the song maps are emitted elsewhere.
#include "rb_meta/persistentdata/SongPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/streams/RevisionStream.h"

// Reconstructed from eboot.elf at 0xD69800.
SongPersistentData::SongPersistentData(RBProfile* profile, int songId)
    : mLoadedRevision(0), mSongID(songId), mLastPlayed(0), mPlayCount(0) {
    mParent = profile;
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    _ForEachScore([](ScoreType, Difficulty, int& score) { score = 0; });
    _ForEachStar([](ScoreType, Difficulty, signed char& stars) { stars = 0; });
    _ForEachAccuracy([](ScoreType, Difficulty, signed char& accuracy) { accuracy = 0; });
    for (int type = 0; type < kAllScores; ++type) {
        for (int difficulty = 0; difficulty < kNumDifficulties; ++difficulty) {
            mScoreEligibility[type][difficulty] = 0;
        }
    }
    ClearLastPerformance();
    if (profile != nullptr) {
        MakeDirty();
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD69AA0.
void SongPersistentData::_ForEachScore(std::function<void(ScoreType, Difficulty, int&)> function) const {
    SongPersistentData* const self = const_cast<SongPersistentData*>(this);
    for (int type = 0; type < kAllScores; ++type) {
        for (int difficulty = 0; difficulty < kNumDifficulties; ++difficulty) {
            function(static_cast<ScoreType>(type), static_cast<Difficulty>(difficulty),
                self->mScores[type][difficulty]);
        }
    }
}

// Reconstructed from eboot.elf at 0xD69BC0.
void SongPersistentData::_ForEachStar(std::function<void(ScoreType, Difficulty, signed char&)> function) const {
    SongPersistentData* const self = const_cast<SongPersistentData*>(this);
    for (int type = 0; type < kAllScores; ++type) {
        for (int difficulty = 0; difficulty < kNumDifficulties; ++difficulty) {
            function(static_cast<ScoreType>(type), static_cast<Difficulty>(difficulty),
                self->mStars[type][difficulty]);
        }
    }
}

// Reconstructed from eboot.elf at 0xD69CE0.
void SongPersistentData::_ForEachAccuracy(
    std::function<void(ScoreType, Difficulty, signed char&)> function) const {
    SongPersistentData* const self = const_cast<SongPersistentData*>(this);
    for (int type = 0; type < kAllScores; ++type) {
        for (int difficulty = 0; difficulty < kNumDifficulties; ++difficulty) {
            function(static_cast<ScoreType>(type), static_cast<Difficulty>(difficulty),
                self->mAccuracies[type][difficulty]);
        }
    }
}

// Reconstructed from eboot.elf at 0xD69E00.
void SongPersistentData::ClearLastPerformance() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    for (int type = 0; type < kAllScores; ++type) {
        mLastPerformanceValues[0][type] = 0;
        mLastPerformanceValues[1][type] = 0;
        mLastPerformanceValues[2][type] = 0;
        mLastPerformanceValues[3][type] = 0;
        mLastPerformanceValues[4][type] = kNumDifficulties;
        mLastPerformanceMarks[0][type] = 0;
        mLastPerformanceMarks[1][type] = 0;
        mLastPerformanceMarks[2][type] = 0;
        mLastPerformanceMarks[3][type] = 0;
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6A080. The assignment is inlined.
SongPersistentData::SongPersistentData(const SongPersistentData& other) {
    mParent = other.mParent;
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    *this = other;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6A160.
SongPersistentData& SongPersistentData::operator=(const SongPersistentData& other) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    _Copy(other);
    MemPopHeap();
    return *this;
}

// Reconstructed from eboot.elf at 0xD6A1D0.
void SongPersistentData::_Copy(const SongPersistentData& other) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mSongID = other.mSongID;
    mLastPlayed = other.mLastPlayed;
    mPlayCount = other.mPlayCount;
    for (int type = 0; type < kAllScores; ++type) {
        for (int row = 0; row < 5; ++row) {
            mLastPerformanceValues[row][type] = other.mLastPerformanceValues[row][type];
        }
        for (int row = 0; row < 4; ++row) {
            mLastPerformanceMarks[row][type] = other.mLastPerformanceMarks[row][type];
        }
    }
    _ForEachScore([&other](ScoreType type, Difficulty difficulty, int& score) {
        score = other.mScores[type][difficulty];
    });
    _ForEachStar([&other](ScoreType type, Difficulty difficulty, signed char& stars) {
        stars = other.mStars[type][difficulty];
    });
    _ForEachAccuracy([&other](ScoreType type, Difficulty difficulty, signed char& accuracy) {
        accuracy = other.mAccuracies[type][difficulty];
    });
    for (int type = 0; type < kAllScores; ++type) {
        for (int difficulty = 0; difficulty < kNumDifficulties; ++difficulty) {
            mScoreEligibility[type][difficulty] = other.mScoreEligibility[type][difficulty];
        }
    }
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6A8F0. The deleting destructor is at
// 0xD6A900.
SongPersistentData::~SongPersistentData() {}

// Reconstructed from eboot.elf at 0xD6A910.
void SongPersistentData::MakeDirty() {
    if (mParent != nullptr) {
        mParent->MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD6A920.
void SongPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 8, 0);
        revisionStream.BeginWrite();
        int value = mSongID;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mPlayCount;
        revisionStream.WriteEndian(&value, sizeof(value));
        revisionStream << mLastPlayed;
        BinStream& out = revisionStream;
        _ForEachScore([&out](ScoreType, Difficulty, int& score) {
            int written = score;
            out.WriteEndian(&written, sizeof(written));
        });
        _ForEachStar([&out](ScoreType, Difficulty, signed char& stars) {
            signed char written = stars;
            out.Write(&written, sizeof(written));
        });
        _ForEachAccuracy([&out](ScoreType, Difficulty, signed char& accuracy) {
            signed char written = accuracy;
            out.Write(&written, sizeof(written));
        });
        for (int type = 0; type < kAllScores; ++type) {
            for (int difficulty = 0; difficulty < kNumDifficulties; ++difficulty) {
                signed char mark = mScoreEligibility[type][difficulty];
                revisionStream.Write(&mark, sizeof(mark));
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6AD10. Revisions up to 6 had a flag
// and revisions 6 and 7 seven bytes, which are read and dropped; revision 1
// kept 64 other values instead of the scores and stars.
void SongPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 8, 0);
        if (revisionStream.BeginRead()) {
            const int revision = revisionStream.mReadRevision;
            mLoadedRevision = revision;
            revisionStream.ReadEndian(&mSongID, sizeof(mSongID));
            revisionStream.ReadEndian(&mPlayCount, sizeof(mPlayCount));
            revisionStream >> mLastPlayed;
            bool readScores = true;
            if (revisionStream.mReadRevision <= 6) {
                unsigned char droppedFlag;
                revisionStream.Read(&droppedFlag, sizeof(droppedFlag));
                if (revisionStream.mReadRevision == 1) {
                    int droppedValue;
                    for (int i = 0; i < 32; ++i) {
                        revisionStream.ReadEndian(&droppedValue, sizeof(droppedValue));
                    }
                    for (int i = 0; i < 32; ++i) {
                        revisionStream.ReadEndian(&droppedValue, sizeof(droppedValue));
                    }
                    readScores = false;
                }
            }
            BinStream& in = revisionStream;
            if (readScores) {
                _ForEachScore([&in](ScoreType, Difficulty, int& score) { in.ReadEndian(&score, sizeof(score)); });
                _ForEachStar([&in](ScoreType, Difficulty, signed char& stars) { in.Read(&stars, sizeof(stars)); });
            }
            if (revisionStream.mReadRevision >= 3) {
                _ForEachAccuracy([&in](ScoreType, Difficulty, signed char& accuracy) {
                    in.Read(&accuracy, sizeof(accuracy));
                });
                if (revisionStream.mReadRevision >= 5) {
                    for (int type = 0; type < kAllScores; ++type) {
                        for (int difficulty = 0; difficulty < kNumDifficulties; ++difficulty) {
                            revisionStream.Read(&mScoreEligibility[type][difficulty], 1);
                        }
                    }
                    if ((revisionStream.mReadRevision & ~1) == 6) {
                        unsigned char droppedMark;
                        for (int i = 0; i < kAllScores; ++i) {
                            revisionStream.Read(&droppedMark, sizeof(droppedMark));
                        }
                    }
                }
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6B7E0.
int SongPersistentData::GetScore(ScoreType type, Difficulty difficulty) const {
    return mScores[type][difficulty];
}

// Reconstructed from eboot.elf at 0xD6B800.
void SongPersistentData::SetLastPlayedToNow() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mLastPlayed = GetDateAndTime();
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6B880.
void SongPersistentData::IncrementPlayCount() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (mPlayCount < 999999999) {
        ++mPlayCount;
    }
    MakeDirty();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6B900. GetHighScore is inlined.
bool SongPersistentData::_UpdateBestResult(
    ScoreType type,
    Difficulty difficulty,
    int score,
    int leaderboardScore,
    signed char stars,
    signed char accuracy,
    bool eligible) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    _UpdateScoreEligibility(type, difficulty, score, eligible);
    int& bestScore = mScores[type][difficulty];
    signed char& bestStars = mStars[type][difficulty];
    signed char& bestAccuracy = mAccuracies[type][difficulty];
    const bool newGold = stars == 6 && bestStars < stars;
    bool improved = false;
    if (bestScore < score || newGold
        || (bestScore == score && (bestStars < stars || (bestStars == stars && bestAccuracy < accuracy)))) {
        bestScore = score;
        bestStars = stars;
        bestAccuracy = accuracy;
        MakeDirty();
        improved = true;
    }
    int highScore;
    Difficulty highDifficulty;
    GetHighScore(type, highScore, highDifficulty);
    if (leaderboardScore < score || leaderboardScore < highScore) {
        MakeDirty();
    }
    MemPopHeap();
    return improved;
}

// Reconstructed from eboot.elf at 0xD6BB30.
signed char SongPersistentData::GetScoreEligibility(ScoreType type, Difficulty difficulty) const {
    return mScoreEligibility[type][difficulty];
}

// Reconstructed from eboot.elf at 0xD6BB50.
void SongPersistentData::GetHighScore(ScoreType type, int& score, Difficulty& difficulty) const {
    score = 0;
    difficulty = kDifficultyEasy;
    _ForEachScore([&type, &score, &difficulty](ScoreType partType, Difficulty partDifficulty, int& partScore) {
        if (partType == type && partScore > 0 && partScore >= score) {
            score = partScore;
            difficulty = partDifficulty;
        }
    });
}

// Reconstructed from eboot.elf at 0xD6BC50.
void SongPersistentData::GetHighStars(ScoreType type, int& stars, Difficulty& difficulty) const {
    stars = 0;
    difficulty = kDifficultyEasy;
    _ForEachStar([&type, &stars, &difficulty](ScoreType partType, Difficulty partDifficulty, signed char& partStars) {
        if (partType == type && partStars > 0 && partStars >= stars) {
            stars = partStars;
            difficulty = partDifficulty;
        }
    });
}

// Reconstructed from eboot.elf at 0xD6BD50.
int SongPersistentData::GetHighStatStars() const {
    int stars = 0;
    _ForEachStar([&stars](ScoreType, Difficulty, signed char& partStars) {
        if (stars < partStars) {
            stars = partStars;
        }
    });
    return stars < 6 ? stars : 5;
}

// Reconstructed from eboot.elf at 0xD6BE20.
int SongPersistentData::GetCareerScore() const {
    int score = 0;
    _ForEachScore([&score](ScoreType type, Difficulty, int& partScore) {
        if (type != kScoreBand && score < partScore) {
            score = partScore;
        }
    });
    return score;
}

// Reconstructed from eboot.elf at 0xD6BEE0.
int SongPersistentData::GetPlayCount() const {
    return mPlayCount;
}

// Reconstructed from eboot.elf at 0xD6BEF0.
const DateTime& SongPersistentData::GetLastPlayed() const {
    return mLastPlayed;
}

// Reconstructed from eboot.elf at 0xD6BF00.
int SongPersistentData::GetID() const {
    return mSongID;
}

// Reconstructed from eboot.elf at 0xD6BF10.
bool SongPersistentData::HasScore(ScoreType type) const {
    return mScores[type][kDifficultyEasy] != 0 || mScores[type][kDifficultyMedium] != 0
        || mScores[type][kDifficultyHard] != 0 || mScores[type][kDifficultyExpert] != 0;
}

// Reconstructed from eboot.elf at 0xD6BF40.
signed char SongPersistentData::GetStars(ScoreType type, Difficulty difficulty) const {
    return mStars[type][difficulty];
}

// Reconstructed from eboot.elf at 0xD6BF60.
signed char SongPersistentData::GetAccuracy(ScoreType type, Difficulty difficulty) const {
    return mAccuracies[type][difficulty];
}

// Reconstructed from eboot.elf at 0xD6BF80.
bool SongPersistentData::IsBestScoreEligible(ScoreType type, Difficulty difficulty) const {
    return mScoreEligibility[type][difficulty] == 2;
}

// Reconstructed from eboot.elf at 0xD6BFA0.
bool SongPersistentData::HasEligibleScore(ScoreType type, Difficulty difficulty) const {
    return mScoreEligibility[type][difficulty] != 0;
}

// Reconstructed from eboot.elf at 0xD6BFC0.
void SongPersistentData::_UpdateScoreEligibility(ScoreType type, Difficulty difficulty, int score, bool eligible) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    signed char& mark = mScoreEligibility[type][difficulty];
    const int bestScore = mScores[type][difficulty];
    if (eligible) {
        if (bestScore > score) {
            if (mark == 0) {
                mark = 1;
                MakeDirty();
            }
        } else if (mark != 2) {
            mark = 2;
            MakeDirty();
        }
    } else if (bestScore < score && mark == 2) {
        mark = 1;
        MakeDirty();
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6C0A0.
QuickplaySongPersistentData::QuickplaySongPersistentData(RBProfile* profile, int songId)
    : SongPersistentData(profile, songId) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mChecksum.Clear();
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6C120. The assignment is inlined.
QuickplaySongPersistentData::QuickplaySongPersistentData(const QuickplaySongPersistentData& other)
    : SongPersistentData(other) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    *this = other;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6C210.
QuickplaySongPersistentData& QuickplaySongPersistentData::operator=(const QuickplaySongPersistentData& other) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    _Copy(other);
    mChecksum = other.mChecksum;
    MakeDirty();
    MemPopHeap();
    return *this;
}

// Reconstructed from eboot.elf at 0xD6C2B0.
void QuickplaySongPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    SongPersistentData::Save(stream);
    {
        RevisionStream revisionStream(stream, 3, 0);
        revisionStream.BeginWrite();
        revisionStream.Write(mChecksum.mHash, sizeof(mChecksum.mHash));
        revisionStream.Write(mChecksum.mPartVersions, sizeof(mChecksum.mPartVersions));
        revisionStream.Write(mChecksum.mPartChecksums, sizeof(mChecksum.mPartChecksums));
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6C3B0. Before revision 2 one version
// byte stood for every part (none at revision 0); before revision 3 the
// part checksums are recomputed.
void QuickplaySongPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    SongPersistentData::Load(stream);
    if (mLoadedRevision >= 4) {
        RevisionStream revisionStream(stream, 3, 0);
        if (revisionStream.BeginRead()) {
            revisionStream.Read(mChecksum.mHash, sizeof(mChecksum.mHash));
            if (revisionStream.mReadRevision >= 2) {
                revisionStream.Read(mChecksum.mPartVersions, sizeof(mChecksum.mPartVersions));
            } else if (revisionStream.mReadRevision == 1) {
                unsigned char version;
                revisionStream.Read(&version, sizeof(version));
                for (int type = 0; type < kAllScores; ++type) {
                    mChecksum.mPartVersions[type] = version;
                }
            } else {
                for (int type = 0; type < kAllScores; ++type) {
                    mChecksum.mPartVersions[type] = 0;
                }
            }
            if (revisionStream.mReadRevision >= 3) {
                revisionStream.Read(mChecksum.mPartChecksums, sizeof(mChecksum.mPartChecksums));
            } else {
                mChecksum.ComputePartChecksums(mSongID);
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6C550. The results are cleared before
// the new fingerprint is kept, without marking the data dirty.
void QuickplaySongPersistentData::SetChecksum(const SongChecksum& checksum) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (!mChecksum.Matches(checksum)) {
        const SongChecksum& saved = mChecksum;
        _ForEachScore([&saved, &checksum](ScoreType type, Difficulty, int& score) {
            if (!saved.PartMatches(checksum, type)) {
                score = 0;
            }
        });
        _ForEachAccuracy([&saved, &checksum](ScoreType type, Difficulty, signed char& accuracy) {
            if (!saved.PartMatches(checksum, type)) {
                accuracy = 0;
            }
        });
        _ForEachStar([&saved, &checksum](ScoreType type, Difficulty, signed char& stars) {
            if (!saved.PartMatches(checksum, type)) {
                stars = 0;
            }
        });
    }
    mChecksum = checksum;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD6C800.
int SongPersistentData::GetSaveSize() {
    return 210;
}

// Reconstructed from eboot.elf at 0xD6C810.
int QuickplaySongPersistentData::GetSaveSize() {
    return 266;
}

// Reconstructed from eboot.elf at 0xD6C820.
void SongPersistentData::SetChecksum(const SongChecksum&) {}

// Reconstructed from eboot.elf at 0xD6C830. The deleting destructor is at
// 0xD6C840.
QuickplaySongPersistentData::~QuickplaySongPersistentData() {}
