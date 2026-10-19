#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/options/SystemOptions.h"
#include "rb_meta/songs/RBSongSort.h"
#include "utl/text/Str.h"

class BinStream;

// The gameplay modifiers in effect: the console's options, or those of the
// online session that overrides them (the game's lookup at 0x9AE5C0 returns
// the session's copy at +800 of its object, else the system options'). An
// interface without a destructor; its vtable in RBSystemOptions is at
// 0x197E1B8. Name and method names not in the reference map; the evidence
// is the fields RBSystemOptions returns.
class IGameplayModifierOptions {
public:
    virtual bool GetIndependentTrackSpeed() const = 0;  // slot 0
    virtual bool GetNoFail() const = 0;                 // slot 1
};

// The options a player brings to gameplay: the console's for local
// players, a remote player's own copy otherwise (the lookup at 0x9AE5F0
// returns +232 of the player's user, else the system options'). An
// interface without a destructor; its vtable in RBSystemOptions is at
// 0x197E1D8. Name and method names not in the reference map; the evidence
// is the fields RBSystemOptions returns.
class IPlayerGameplayOptions {
public:
    virtual unsigned int GetProDrumCymbalLanes() const = 0;  // slot 0
    virtual bool GetAutokick() const = 0;                     // slot 1
    virtual bool GetImprovSolosScored() const = 0;            // slot 2
    virtual float GetAudioOffset() const = 0;                 // slot 3
    virtual float GetVideoOffset() const = 0;                 // slot 4
};

// The game's console-wide options (rb_meta/RBSystemOptions.o,
// 0xD663F0-0xD68800): SystemOptions' calibration plus the volumes, the
// options screen's checkboxes, the song library's filters and sort, and a
// few flags the shell remembers. RBProfileMgr keeps the one instance. The
// save is a revision-28 block after SystemOptions' own and before the song
// manager's cache. Every setter stores the value, under the "profile"
// heap, only when it changes and then marks the options dirty.
//
// The vtable group at 0x197E0D0 has the primary vtable (0x197E0E0, 16
// slots), SystemOptions' MsgSource vtable (0x197E170, offset 24) and the
// two interfaces' (0x197E1B8 at offset 216, 0x197E1D8 at offset 224). The
// object is 336 bytes. The map's build is older: it has no interfaces and
// fewer options.
class RBSystemOptions : public SystemOptions, public IGameplayModifierOptions, public IPlayerGameplayOptions {
public:
    DELETE_OVERLOAD

    // The revision Save writes. Name not in the reference map.
    static constexpr int kRevision = 28;
    // The volume sliders' steps run from 0 to this index, 0 dB. Name not in
    // the reference map.
    static constexpr unsigned int kMaxVolumeIndex = 11;

    RBSystemOptions();  // 0xD663F0
    // Slots 0-1: 0xD68370, 0xD68460; MsgSource thunks 0xD685C0, 0xD686A0.
    ~RBSystemOptions() override;

    // Slot 5: SystemOptions' block, then this one's, then the song cache.
    void Save(BinStream& stream) const override;  // 0xD667C0
    // Slot 6: reads the values the stored revision has, clamps the volumes,
    // skips the data of dropped revisions and marks the options clean.
    void Load(BinStream& stream) override;  // 0xD66B70
    // Slot 8: restores SystemOptions' defaults, the cymbal lanes and the
    // song library and shell settings; the volumes and the other checkboxes
    // keep their values.
    void Reset() override;  // 0xD66500

    // Slots 9-15, the interfaces' overriders; the interface slots are
    // thunks at 0xD68790-0xD687F0. Names not in the reference map.
    bool GetNoFail() const override;                  // 0xD68550
    bool GetIndependentTrackSpeed() const override;   // 0xD68560
    unsigned int GetProDrumCymbalLanes() const override;  // 0xD68570
    bool GetImprovSolosScored() const override;       // 0xD68580
    bool GetAutokick() const override;                // 0xD68590
    float GetAudioOffset() const override;            // 0xD685A0
    float GetVideoOffset() const override;            // 0xD685B0

    // Volume slider steps.
    void SetBackgroundVolume(unsigned int volume);   // 0xD67570
    void SetInstrumentsVolume(unsigned int volume);  // 0xD675F0
    void SetSFXVolume(unsigned int volume);          // 0xD67670
    void SetCrowdVolume(unsigned int volume);        // 0xD676F0
    // The volumes in decibels: the background and instrument steps from
    // -8 dB, the SFX and crowd steps from -18 dB, each up to 0 dB.
    float GetBackgroundVolumeDb() const;   // 0xD67770
    float GetInstrumentsVolumeDb() const;  // 0xD67790
    float GetSFXVolumeDb() const;          // 0xD677B0
    float GetCrowdVolumeDb() const;        // 0xD677D0

    // The options screen's checkboxes (RBSystemOptionsCom's kCheckbox*).
    // SetSubtitles, SetLeaderboardLadder and SetAutokick are not in the
    // reference map; they follow the checkboxes' names.
    void SetBassBoost(bool bassBoost);                    // 0xD677F0
    void SetDolby(bool dolby);                            // 0xD67870
    // Hides SystemOptions::SetOverscan(float, bool), as in the map.
    void SetOverscan(bool overscan);                      // 0xD678F0
    void SetIsDrumNavigationAllowed(bool allowed);        // 0xD67970
    void SetNoFail(bool noFail);                          // 0xD679F0
    void SetIndependentTrackSpeed(bool independent);      // 0xD67A70
    void SetProDrumCymbalLanes(unsigned int lanes);       // 0xD67AF0
    void SetImprovSolosScored(bool scored);               // 0xD67B70
    void SetSubtitles(bool subtitles);                    // 0xD67BF0
    void SetAwesomenessDetection(bool detection);         // 0xD67C70
    void SetLeaderboardLadder(bool ladder);               // 0xD67CF0
    void SetSongLibSort(RBSongSort::SortMethod sort);     // 0xD67D70
    // The song library's filter checkboxes (RBSongLibFilterCategoriesProvider
    // toggles them at 0xC7AEA0). Names not in the reference map.
    void SetHide1RatedSongs(bool hide);                   // 0xD67DF0
    void SetHideUnavailableSongs(bool hide);              // 0xD67E70
    void SetRivalsChallengeFilter(bool filter);           // 0xD67EF0
    // The song library's stats and leaderboard modes. Nothing in the binary
    // calls these two. Names not in the reference map; the evidence is
    // weak.
    void SetSongLibStatsMode(unsigned int mode);          // 0xD67F70
    void SetSongLibLeaderboardMode(unsigned int mode);    // 0xD67FF0
    void SetAutoShowLeaderboard(bool show);               // 0xD68070
    // The last "current_patch" of the "patch_versions" config the player
    // was told about (0xCAAC27). Name not in the reference map.
    void SetLastSeenPatch(unsigned int patch);            // 0xD680F0
    // Set once the startup upsell was offered (0xB56AAF). Name not in the
    // reference map; the evidence is weak.
    void SetHasSeenUpsell(bool seen);                     // 0xD68170
    void SetAutokick(bool autokick);                      // 0xD681F0
    // The graphics mode checkboxes (0 1080p30, 1 high-res 30, 2 1080p60,
    // 3 1080p30 high detail) and HDR. Names not in the reference map.
    void SetGraphicsMode(unsigned int mode);              // 0xD68270
    void SetHDR(unsigned int hdr);                        // 0xD682F0

    // The two texts Reset clears and the revision-17 save carries. Nothing
    // else in the binary reads or writes them, so their purpose is not
    // known; the names only describe them. Names not in the reference map.
    void SetStoredText(const char* text);   // 0xD665E0
    void SetStoredText2(const char* text);  // 0xD666D0

    // The slider step's entry in the decibel table. Inlined into the
    // getters in this build.
    float _SliderIndexToDb(unsigned int index, const float* table) const {
        return table[index];
    }

    // Field names are not in the reference map; the setters name them.
    unsigned int mBackgroundVolume;
    unsigned int mInstrumentsVolume;
    unsigned int mSFXVolume;
    unsigned int mCrowdVolume;
    bool mDolby;
    bool mBassBoost;
    // Named apart from SystemOptions' overscan scale.
    bool mOverscanEnabled;
    bool mIsDrumNavigationAllowed;
    // A mask of the cymbal lanes; 0x1C by default.
    unsigned int mProDrumCymbalLanes;
    bool mNoFail;
    bool mIndependentTrackSpeed;
    bool mImprovSolosScored;
    bool mAwesomenessDetection;
    bool mLeaderboardLadder;
    bool mAutokick;
    bool mSubtitles;
    bool mHide1RatedSongs;
    bool mHideUnavailableSongs;
    bool mRivalsChallengeFilter;
    RBSongSort::SortMethod mSongLibSort;
    // 1 (global) by default; 0 is the rivals challenge.
    unsigned int mSongLibStatsMode;
    // 0 (none) by default.
    unsigned int mSongLibLeaderboardMode;
    bool mAutoShowLeaderboard;
    unsigned int mLastSeenPatch;
    bool mHasSeenUpsell;
    String mStoredText;
    String mStoredText2;
    unsigned int mGraphicsMode;
    // 1 when HDR is on.
    unsigned int mHDR;
};

static_assert(offsetof(RBSystemOptions, mBackgroundVolume) == 232);
static_assert(offsetof(RBSystemOptions, mSFXVolume) == 240);
static_assert(offsetof(RBSystemOptions, mCrowdVolume) == 244);
static_assert(offsetof(RBSystemOptions, mDolby) == 248);
static_assert(offsetof(RBSystemOptions, mProDrumCymbalLanes) == 252);
static_assert(offsetof(RBSystemOptions, mNoFail) == 256);
static_assert(offsetof(RBSystemOptions, mRivalsChallengeFilter) == 265);
static_assert(offsetof(RBSystemOptions, mSongLibSort) == 268);
static_assert(offsetof(RBSystemOptions, mSongLibStatsMode) == 272);
static_assert(offsetof(RBSystemOptions, mAutoShowLeaderboard) == 280);
static_assert(offsetof(RBSystemOptions, mLastSeenPatch) == 284);
static_assert(offsetof(RBSystemOptions, mHasSeenUpsell) == 288);
static_assert(offsetof(RBSystemOptions, mStoredText) == 296);
static_assert(offsetof(RBSystemOptions, mStoredText2) == 312);
static_assert(offsetof(RBSystemOptions, mGraphicsMode) == 328);
static_assert(offsetof(RBSystemOptions, mHDR) == 332);
static_assert(sizeof(RBSystemOptions) == 336);
