#pragma once

#include <cstddef>
#include <functional>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "rb_beatmatch/core/Defines.h"
#include "rb_game/core/Defines.h"
#include "utl/time/DateTime.h"

class BinStream;
class RBPlayer;
class RBProfile;

// The fingerprint of the song version a quickplay result was played on: a
// 20-byte hash and, per score type, a version byte and a checksum (zero when
// unknown). Its code lies outside rb_meta/SongPersistentData.o (from
// 0xF0B930, its object is not identified); only the members the saved song
// data uses are declared. The map's QuickplaySongPersistentData::SetChecksum
// took the hash alone. Class, member and field names are not in the
// reference map.
class SongChecksum {
public:
    // Zeroes the fingerprint.
    void Clear();  // 0xF0B930
    // Whether the two fingerprints describe the same song version: the
    // version bytes are equal and the checksums equal where both are known.
    bool Matches(const SongChecksum& other) const;  // 0xF0BCA0
    // The same test for one score type.
    bool PartMatches(const SongChecksum& other, ScoreType type) const;  // 0xF0BD80
    // Recomputes the checksums from the song (through the global at
    // 0x1B43A98, 0xF33AB0), for results saved before revision 3.
    void ComputePartChecksums(int songId);  // 0xF0BDB0

    unsigned char mHash[20];
    unsigned char mPartVersions[kAllScores];
    int mPartChecksums[kAllScores];
};

static_assert(offsetof(SongChecksum, mPartVersions) == 20);
static_assert(offsetof(SongChecksum, mPartChecksums) == 28);
static_assert(sizeof(SongChecksum) == 56);

// A song's saved results for a profile (rb_meta/SongPersistentData.o,
// 0xD69800-0xD6DD6F): per score type and difficulty the best score with its
// stars and accuracy and whether an eligible score was reached, the play
// count, when it was last played, and the results of the last performance
// (not saved). The vtable at 0x197E2C8 has PersistentData's slots and an
// empty eighth slot, SetChecksum, that QuickplaySongPersistentData
// overrides; the base's slot is newer than the map. The object is 408
// bytes.
class SongPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    SongPersistentData(RBProfile* profile = nullptr, int songId = 0);  // 0xD69800
    // Copies the results with _Copy, which marks the copy dirty. Not in the
    // reference map, nor the assignment.
    SongPersistentData(const SongPersistentData& other);  // 0xD6A080
    SongPersistentData& operator=(const SongPersistentData& other);  // 0xD6A160
    // Slots 0-1: 0xD6A8F0, 0xD6A900.
    ~SongPersistentData() override;

    // Slot 2: marks the profile dirty. The map's version also flagged the
    // song for a leaderboard upload (IsUploadDirty, ClearUploadDirty), which
    // this build no longer has.
    void MakeDirty() override;  // 0xD6A910
    // Slot 5: a revision-8 block.
    void Save(BinStream& stream) const override;  // 0xD6A920
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD6AD10
    // Slot 7: empty here. The map's QuickplaySongPersistentData::SetChecksum
    // takes unsigned char const*.
    virtual void SetChecksum(const SongChecksum& checksum);  // 0xD6C820

    // Records a finished performance: the player's part and the band's
    // last-performance results and best results, the play count and date,
    // and the song's checksum (slot 7); the flags tell whether each best
    // result improved. Not reconstructed: it needs RBPlayer, BandUser, the
    // game data and the leaderboard cache, which are not modelled. The map
    // has Update(RBPlayer const&, bool&, bool&).
    void Update(const RBPlayer& player, const SongChecksum& checksum, bool& newBest, bool& newBandBest);  // 0xD6B320
    // The score of a type and difficulty.
    int GetScore(ScoreType type, Difficulty difficulty) const;  // 0xD6B7E0
    void SetLastPlayedToNow();  // 0xD6B800
    // Counts the play, up to 999,999,999.
    void IncrementPlayCount();  // 0xD6B880
    // Keeps the better of the stored and the new result of a type and
    // difficulty (a higher score, new gold stars, or the same score with more
    // stars or the same stars and a better accuracy), and marks the data
    // dirty also when the score or the best score of the type beats the
    // leaderboard's. True when the result improved. Name not in the
    // reference map; the leaderboard reading of `leaderboardScore` is weak.
    bool _UpdateBestResult(
        ScoreType type,
        Difficulty difficulty,
        int score,
        int leaderboardScore,
        signed char stars,
        signed char accuracy,
        bool eligible);  // 0xD6B900
    // The eligibility mark of a type and difficulty (see
    // mScoreEligibility). Name not in the reference map.
    signed char GetScoreEligibility(ScoreType type, Difficulty difficulty) const;  // 0xD6BB30
    // The best positive score of the type and its difficulty, the hardest on
    // a tie; zeroes when there is none.
    void GetHighScore(ScoreType type, int& score, Difficulty& difficulty) const;  // 0xD6BB50
    // The same for the stars.
    void GetHighStars(ScoreType type, int& stars, Difficulty& difficulty) const;  // 0xD6BC50
    // The most stars of any part, gold stars (6) counting as 5. The evidence
    // for the map's name is weak.
    int GetHighStatStars() const;  // 0xD6BD50
    // The best score of any part but the band's, which the profile adds up
    // for its career score. Name not in the reference map; the evidence for
    // it is weak.
    int GetCareerScore() const;  // 0xD6BE20
    int GetPlayCount() const;  // 0xD6BEE0
    const DateTime& GetLastPlayed() const;  // 0xD6BEF0
    int GetID() const;  // 0xD6BF00
    // Whether the type has a score on any difficulty. Name not in the
    // reference map.
    bool HasScore(ScoreType type) const;  // 0xD6BF10
    signed char GetStars(ScoreType type, Difficulty difficulty) const;     // 0xD6BF40
    signed char GetAccuracy(ScoreType type, Difficulty difficulty) const;  // 0xD6BF60
    // Whether the best score was eligible, and whether any eligible score
    // was reached. Names not in the reference map.
    bool IsBestScoreEligible(ScoreType type, Difficulty difficulty) const;  // 0xD6BF80
    bool HasEligibleScore(ScoreType type, Difficulty difficulty) const;     // 0xD6BFA0
    // Updates the eligibility mark for a new score. Name not in the
    // reference map.
    void _UpdateScoreEligibility(ScoreType type, Difficulty difficulty, int score, bool eligible);  // 0xD6BFC0
    // The bytes Save writes: 210. Unreferenced; name not in the reference
    // map, the reading of the value is weak.
    static int GetSaveSize();  // 0xD6C800

    // Runs the function on the result of each score type and difficulty.
    // The map has a const and a non-const overload of each; this build
    // emits one body, which both use.
    void _ForEachScore(std::function<void(ScoreType, Difficulty, int&)> function) const;  // 0xD69AA0
    void _ForEachStar(std::function<void(ScoreType, Difficulty, signed char&)> function) const;  // 0xD69BC0
    void _ForEachAccuracy(std::function<void(ScoreType, Difficulty, signed char&)> function) const;  // 0xD69CE0
    // Resets the last performance. Name not in the reference map.
    void ClearLastPerformance();  // 0xD69E00
    // Copies the results, not the revision, and marks the data dirty. Name
    // not in the reference map.
    void _Copy(const SongPersistentData& other);  // 0xD6A1D0

    // Field names are not in the reference map.
    // The revision Load read; QuickplaySongPersistentData reads its own
    // block from revision 4.
    int mLoadedRevision;
    int mSongID;
    DateTime mLastPlayed;
    int mScores[kAllScores][kNumDifficulties];
    signed char mStars[kAllScores][kNumDifficulties];
    signed char mAccuracies[kAllScores][kNumDifficulties];
    // 0 when no eligible score was reached, 1 when one was but the best
    // score was not, 2 when the best score was. Eligibility is what the
    // player's 0xA5D580 reports (all players for the band); the reading is
    // weak.
    signed char mScoreEligibility[kAllScores][kNumDifficulties];
    int mPlayCount;
    // The last performance per score type, which Update fills: rows 1 and 2
    // two of the player's scores (0xA5DA00 with false and true, the second
    // the one the best results take), row 3 another of its values (0xA5FA00)
    // and row 4 the difficulty (kNumDifficulties when none; the easiest of
    // the band). Update leaves row 0 alone. The bytes are the stars, the
    // accuracy, the eligibility mark and whether the performance was
    // eligible. The reading of the rows is weak.
    int mLastPerformanceValues[5][kAllScores];
    signed char mLastPerformanceMarks[4][kAllScores];
};

static_assert(offsetof(SongPersistentData, mLoadedRevision) == 24);
static_assert(offsetof(SongPersistentData, mSongID) == 28);
static_assert(offsetof(SongPersistentData, mLastPlayed) == 32);
static_assert(offsetof(SongPersistentData, mScores) == 40);
static_assert(offsetof(SongPersistentData, mStars) == 152);
static_assert(offsetof(SongPersistentData, mAccuracies) == 180);
static_assert(offsetof(SongPersistentData, mScoreEligibility) == 208);
static_assert(offsetof(SongPersistentData, mPlayCount) == 236);
static_assert(offsetof(SongPersistentData, mLastPerformanceValues) == 240);
static_assert(offsetof(SongPersistentData, mLastPerformanceMarks) == 380);
static_assert(sizeof(SongPersistentData) == 408);

// A quickplay song's results with the fingerprint of the song version they
// were played on (rb_meta/SongPersistentData.o). The profile keeps them in
// two maps keyed by song id. The vtable is at 0x197E318; the object is 464
// bytes.
class QuickplaySongPersistentData : public SongPersistentData {
public:
    QuickplaySongPersistentData(RBProfile* profile = nullptr, int songId = 0);  // 0xD6C0A0
    QuickplaySongPersistentData(const QuickplaySongPersistentData& other);  // 0xD6C120
    QuickplaySongPersistentData& operator=(const QuickplaySongPersistentData& other);  // 0xD6C210
    // Slots 0-1: 0xD6C830, 0xD6C840.
    ~QuickplaySongPersistentData() override;

    // Slot 5: the base's block, then the fingerprint in a revision-3 block.
    void Save(BinStream& stream) const override;  // 0xD6C2B0
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD6C3B0
    // Slot 7: a different song version clears the results of the parts that
    // changed, then the fingerprint is kept.
    void SetChecksum(const SongChecksum& checksum) override;  // 0xD6C550

    // The bytes Save writes: 266. Unreferenced; name not in the reference
    // map, the reading of the value is weak.
    static int GetSaveSize();  // 0xD6C810

    // Field name not in the reference map.
    SongChecksum mChecksum;
};

static_assert(offsetof(QuickplaySongPersistentData, mChecksum) == 408);
static_assert(sizeof(QuickplaySongPersistentData) == 464);
