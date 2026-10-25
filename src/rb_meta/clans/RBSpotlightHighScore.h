#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

class BinStream;

// A clan's best score on a Band Wars spotlight song for one instrument, as
// the spotlight stats job returns it ("clan_spotlight_stats_job_complete";
// RBBandWarsSpotlightStatsListProviderCom fills them from "song_scores" at
// 0xE2FAFA). The profile caches them per song in
// RBClanPersistentData::mSpotlightScores. Newer than the map: the class and
// field names are inferred from those keys. The three classes are the
// saved layouts: revisions 1-4 of the clan data hold
// RBSpotlightHighScoreV1, revision 5 RBSpotlightHighScoreV2 and revision 6
// RBSpotlightHighScore. Their members are inline; the binary emits them in
// rb_meta/RBClanPersistentData.o. Their stream operators sit in the clan
// code before RBRivalsPlayerProfile's constructor (0xB24A80-0xB24DE8) and
// are declared only.
class RBSpotlightHighScoreV1 {
public:
    RBSpotlightHighScoreV1() {
        RBSpotlightHighScoreV1::Reset();
    }
    // Slots 0-1: 0xD51770 (empty), 0xD51780.
    virtual ~RBSpotlightHighScoreV1() {}
    // Slot 2: 0xD51790.
    virtual void Reset() {
        mHasScore = false;
        mInstrument = 0;
        mHighScore = 0;
        mHighScoreStars = 0;
        mHighScoreDifficulty = 0;
        mHighScoreBrutal = false;
        mHighScorerPlatformUid = Symbol(gNullStr);
        mHighScorerUsername = Symbol(gNullStr);
        mHighScorerIconUrl = Symbol(gNullStr);
        mHighScorerOnlineId = Symbol(gNullStr);
    }

    // Set when the job's entry is read. The name is weak.
    bool mHasScore;
    // "instrument"; read through a temporary, probably an enum.
    int mInstrument;
    int mHighScore;       // "high_score"
    int mHighScoreStars;  // "high_score_stars"
    // "high_score_difficulty"; read through a temporary, probably an enum.
    int mHighScoreDifficulty;
    bool mHighScoreBrutal;           // "high_score_brutal"
    Symbol mHighScorerPlatformUid;   // "high_scorer_platform_uid"
    Symbol mHighScorerUsername;      // "high_scorer_username"
    Symbol mHighScorerIconUrl;       // "high_scorer_icon_url"
    // The provider fills it from "high_scorer_platform_uid" again; the name
    // is weak.
    Symbol mHighScorerOnlineId;
};

static_assert(offsetof(RBSpotlightHighScoreV1, mHasScore) == 8);
static_assert(offsetof(RBSpotlightHighScoreV1, mInstrument) == 12);
static_assert(offsetof(RBSpotlightHighScoreV1, mHighScoreDifficulty) == 24);
static_assert(offsetof(RBSpotlightHighScoreV1, mHighScoreBrutal) == 28);
static_assert(offsetof(RBSpotlightHighScoreV1, mHighScorerPlatformUid) == 32);
static_assert(offsetof(RBSpotlightHighScoreV1, mHighScorerOnlineId) == 56);
static_assert(sizeof(RBSpotlightHighScoreV1) == 64);

// Adds the percentile (revision 5).
class RBSpotlightHighScoreV2 : public RBSpotlightHighScoreV1 {
public:
    // Emitted out of line at 0xD514A0.
    RBSpotlightHighScoreV2() {
        RBSpotlightHighScoreV2::Reset();
    }
    // Slots 0-1: 0xD515E0 (empty), 0xD516B0.
    ~RBSpotlightHighScoreV2() override {}
    // Slot 2: 0xD516C0.
    void Reset() override {
        RBSpotlightHighScoreV1::Reset();
        mHighScorePercentile = 0;
    }

    // "high_score_percentile", rounded and clamped to 0-100.
    int mHighScorePercentile;
};

static_assert(offsetof(RBSpotlightHighScoreV2, mHighScorePercentile) == 64);
static_assert(sizeof(RBSpotlightHighScoreV2) == 72);

// Adds whether the score is the local player's (revision 6).
class RBSpotlightHighScore : public RBSpotlightHighScoreV2 {
public:
    RBSpotlightHighScore() {
        RBSpotlightHighScore::Reset();
    }
    // Slots 0-1: 0xD50C20 (empty), 0xD515F0.
    ~RBSpotlightHighScore() override {}
    // Slot 2: 0xD51600.
    void Reset() override {
        RBSpotlightHighScoreV2::Reset();
        mIsYou = false;
    }

    bool mIsYou;  // "is_you"
};

static_assert(offsetof(RBSpotlightHighScore, mIsYou) == 68);
static_assert(sizeof(RBSpotlightHighScore) == 72);

// Each writes or reads its base's fields first. Reads of the instrument and
// difficulty go through temporaries.
BinStream& operator<<(BinStream& stream, const RBSpotlightHighScoreV1& score);  // 0xB24A80
BinStream& operator>>(BinStream& stream, RBSpotlightHighScoreV1& score);        // 0xB24B80
BinStream& operator<<(BinStream& stream, const RBSpotlightHighScoreV2& score);  // 0xB24C70
BinStream& operator>>(BinStream& stream, RBSpotlightHighScoreV2& score);        // 0xB24CD0
BinStream& operator<<(BinStream& stream, const RBSpotlightHighScore& score);    // 0xB24D00
BinStream& operator>>(BinStream& stream, RBSpotlightHighScore& score);          // 0xB24D80
