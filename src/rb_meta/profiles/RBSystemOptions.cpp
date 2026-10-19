// rb_meta/RBSystemOptions.o (0xD663F0 to 0xD687FF). The interfaces' thunks
// (0xD68790-0xD687F0) and the MsgSource destructor thunks (0xD685C0,
// 0xD686A0) are the compiler's. IDA leaves Save, Load and most setters
// undefined; they are read from the raw code.
#include "rb_meta/profiles/RBSystemOptions.h"

#include <cstring>

#include "os/memory/MemMgr.h"
#include "rb_meta/songs/SongMgr.h"
#include "utl/containers/Map.h"
#include "utl/containers/Set.h"
#include "utl/streams/RevisionStream.h"
#include "utl/text/Symbol.h"

namespace {

// Pushes the "profile" heap for the scope. Each caller keeps its own
// guarded copy of the heap's index. Name not in the reference map.
class ProfileHeapScope {
public:
    explicit ProfileHeapScope(long heap) {
        MemPushHeap(heap);
    }
    ~ProfileHeapScope() {
        MemPopHeap();
    }

    ProfileHeapScope(const ProfileHeapScope&) = delete;
    ProfileHeapScope& operator=(const ProfileHeapScope&) = delete;
};

// The decibels of the background and instrument volume steps (0x13139E0)
// and of the SFX and crowd steps (0x1313A10). Names not in the reference
// map.
const float kMusicVolumeDb[RBSystemOptions::kMaxVolumeIndex + 1] = {
    -8.0f, -7.5f, -6.75f, -6.0f, -5.25f, -4.5f, -3.75f, -3.0f, -2.25f, -1.5f, -0.75f, 0.0f,
};
const float kEffectsVolumeDb[RBSystemOptions::kMaxVolumeIndex + 1] = {
    -18.0f, -15.0f, -13.5f, -12.0f, -10.5f, -9.0f, -7.5f, -6.0f, -4.5f, -3.0f, -1.5f, 0.0f,
};

// The defaults. Names not in the reference map.
constexpr unsigned int kDefaultProDrumCymbalLanes = 0x1C;
constexpr unsigned int kSongLibStatsModeGlobal = 1;
constexpr unsigned int kDefaultGraphicsMode = 1;

// The revisions Load treats specially. Names not in the reference map.
// The song library sort saved before revision 20 used older values.
constexpr int kSongLibSortRevision = 20;
// Revisions 18 to 23 stored a block of song ids and symbols that later
// revisions dropped.
constexpr int kFirstLegacySongDataRevision = 18;
constexpr int kLastLegacySongDataRevision = 23;
// The legacy block's own revision.
constexpr int kLegacySongDataRevision = 2;

}  // namespace

// Reconstructed from eboot.elf at 0xD663F0. Reset runs through the vtable.
RBSystemOptions::RBSystemOptions()
    : mBackgroundVolume(kMaxVolumeIndex),
      mInstrumentsVolume(kMaxVolumeIndex),
      mSFXVolume(kMaxVolumeIndex),
      mCrowdVolume(kMaxVolumeIndex),
      mDolby(false),
      mBassBoost(false),
      mOverscanEnabled(false),
      mIsDrumNavigationAllowed(false),
      mProDrumCymbalLanes(kDefaultProDrumCymbalLanes),
      mNoFail(false),
      mIndependentTrackSpeed(true),
      mImprovSolosScored(true),
      mAwesomenessDetection(false),
      mLeaderboardLadder(true),
      mAutokick(false),
      mSubtitles(false),
      mHide1RatedSongs(false),
      mHideUnavailableSongs(false),
      mRivalsChallengeFilter(false),
      mSongLibSort(static_cast<RBSongSort::SortMethod>(0)),
      mSongLibStatsMode(kSongLibStatsModeGlobal),
      mSongLibLeaderboardMode(0),
      mAutoShowLeaderboard(true),
      mLastSeenPatch(0),
      mHasSeenUpsell(false),
      mStoredText(),
      mStoredText2(),
      mGraphicsMode(kDefaultGraphicsMode),
      mHDR(1) {
    Reset();
}

// Reconstructed from eboot.elf at 0xD68370. The deleting destructor is at
// 0xD68460.
RBSystemOptions::~RBSystemOptions() {}

// Reconstructed from eboot.elf at 0xD66500.
void RBSystemOptions::Reset() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    SystemOptions::Reset();
    mProDrumCymbalLanes = kDefaultProDrumCymbalLanes;
    mSongLibSort = static_cast<RBSongSort::SortMethod>(0);
    mHide1RatedSongs = false;
    mHideUnavailableSongs = false;
    mRivalsChallengeFilter = false;
    mSongLibStatsMode = kSongLibStatsModeGlobal;
    mSongLibLeaderboardMode = 0;
    mAutoShowLeaderboard = true;
    mLastSeenPatch = 0;
    mHasSeenUpsell = false;
    SetStoredText("");
    SetStoredText2("");
}

// Reconstructed from eboot.elf at 0xD665E0.
void RBSystemOptions::SetStoredText(const char* text) {
    if (strcmp(text, mStoredText.c_str()) != 0) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mStoredText = text;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD666D0.
void RBSystemOptions::SetStoredText2(const char* text) {
    if (strcmp(text, mStoredText2.c_str()) != 0) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mStoredText2 = text;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD667C0.
void RBSystemOptions::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    RevisionStream revisionStream(stream, kRevision, 0);
    revisionStream.BeginWrite();
    SystemOptions::Save(revisionStream);
    unsigned int volume = mBackgroundVolume;
    revisionStream.WriteEndian(&volume, sizeof(volume));
    volume = mInstrumentsVolume;
    revisionStream.WriteEndian(&volume, sizeof(volume));
    volume = mSFXVolume;
    revisionStream.WriteEndian(&volume, sizeof(volume));
    volume = mCrowdVolume;
    revisionStream.WriteEndian(&volume, sizeof(volume));
    bool flag = mDolby;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mBassBoost;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mOverscanEnabled;
    revisionStream.Write(&flag, sizeof(flag));
    unsigned int value = mProDrumCymbalLanes;
    revisionStream.WriteEndian(&value, sizeof(value));
    flag = mIsDrumNavigationAllowed;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mNoFail;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mIndependentTrackSpeed;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mImprovSolosScored;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mAwesomenessDetection;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mLeaderboardLadder;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mHide1RatedSongs;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mSubtitles;
    revisionStream.Write(&flag, sizeof(flag));
    int sort = mSongLibSort;
    revisionStream.WriteEndian(&sort, sizeof(sort));
    value = mLastSeenPatch;
    revisionStream.WriteEndian(&value, sizeof(value));
    flag = mHasSeenUpsell;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mAutokick;
    revisionStream.Write(&flag, sizeof(flag));
    revisionStream << mStoredText;
    revisionStream << mStoredText2;
    flag = mHideUnavailableSongs;
    revisionStream.Write(&flag, sizeof(flag));
    flag = mRivalsChallengeFilter;
    revisionStream.Write(&flag, sizeof(flag));
    value = mSongLibStatsMode;
    revisionStream.WriteEndian(&value, sizeof(value));
    value = mSongLibLeaderboardMode;
    revisionStream.WriteEndian(&value, sizeof(value));
    flag = mAutoShowLeaderboard;
    revisionStream.Write(&flag, sizeof(flag));
    value = mGraphicsMode;
    revisionStream.WriteEndian(&value, sizeof(value));
    value = mHDR;
    revisionStream.WriteEndian(&value, sizeof(value));
    theSongMgr->SaveSongCache(revisionStream);
}

// Reconstructed from eboot.elf at 0xD66B70. A block that cannot be read
// changes nothing. SystemOptions' block exists from revision 1. The legacy
// song data of revisions 18 to 23 is read and dropped.
void RBSystemOptions::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    RevisionStream revisionStream(stream, kRevision, 0);
    if (!revisionStream.BeginRead()) {
        return;
    }
    if (revisionStream.mReadRevision > 0) {
        SystemOptions::Load(revisionStream);
    }
    revisionStream >> Revision::AsOfOp<unsigned int>(&mBackgroundVolume, 0);
    revisionStream >> Revision::AsOfOp<unsigned int>(&mInstrumentsVolume, 0);
    revisionStream >> Revision::AsOfOp<unsigned int>(&mSFXVolume, 0);
    revisionStream >> Revision::AsOfOp<unsigned int>(&mCrowdVolume, 0);
    mBackgroundVolume = mBackgroundVolume < kMaxVolumeIndex ? mBackgroundVolume : kMaxVolumeIndex;
    mInstrumentsVolume = mInstrumentsVolume < kMaxVolumeIndex ? mInstrumentsVolume : kMaxVolumeIndex;
    mSFXVolume = mSFXVolume < kMaxVolumeIndex ? mSFXVolume : kMaxVolumeIndex;
    mCrowdVolume = mCrowdVolume < kMaxVolumeIndex ? mCrowdVolume : kMaxVolumeIndex;
    revisionStream >> Revision::AsOfOp<bool>(&mDolby, 0);
    revisionStream >> Revision::AsOfOp<bool>(&mBassBoost, 0);
    revisionStream >> Revision::AsOfOp<bool>(&mOverscanEnabled, 0);
    revisionStream >> Revision::AsOfOp<unsigned int>(&mProDrumCymbalLanes, 2);
    revisionStream >> Revision::AsOfOp<bool>(&mIsDrumNavigationAllowed, 5);
    revisionStream >> Revision::AsOfOp<bool>(&mNoFail, 6);
    revisionStream >> Revision::AsOfOp<bool>(&mIndependentTrackSpeed, 7);
    revisionStream >> Revision::AsOfOp<bool>(&mImprovSolosScored, 8);
    revisionStream >> Revision::AsOfOp<bool>(&mAwesomenessDetection, 10);
    revisionStream >> Revision::AsOfOp<bool>(&mLeaderboardLadder, 14);
    revisionStream >> Revision::AsOfOp<bool>(&mHide1RatedSongs, 19);
    revisionStream >> Revision::AsOfOp<bool>(&mSubtitles, 21);
    int sort = 0;
    revisionStream >> Revision::AsOfOp<int>(&sort, 11);
    mSongLibSort =
        static_cast<RBSongSort::SortMethod>(revisionStream.mReadRevision >= kSongLibSortRevision ? sort : 0);
    // An int that revisions 12 to 15 stored. The map's build still has
    // SetPartyModeSongLibSort(RBSongSort::SortMethod); the evidence that
    // this was that sort is weak.
    int dropped = 0;
    revisionStream >> Revision::RangeOp<int>(&dropped, 12, 15);
    unsigned int lastSeenPatch = 0;
    revisionStream >> Revision::AsOfOp<unsigned int>(&lastSeenPatch, 13);
    mLastSeenPatch = lastSeenPatch;
    revisionStream >> Revision::AsOfOp<bool>(&mHasSeenUpsell, 22);
    revisionStream >> Revision::AsOfOp<bool>(&mAutokick, 15);
    if (revisionStream.mReadRevision <= 8) {
        mImprovSolosScored = true;
    }
    String storedText;
    revisionStream >> Revision::AsOfOp<String>(&storedText, 17);
    SetStoredText(storedText.c_str());
    String storedText2;
    revisionStream >> Revision::AsOfOp<String>(&storedText2, 17);
    SetStoredText2(storedText2.c_str());
    revisionStream >> Revision::AsOfOp<bool>(&mHideUnavailableSongs, 25);
    revisionStream >> Revision::AsOfOp<bool>(&mRivalsChallengeFilter, 26);
    if (revisionStream.mReadRevision >= 28) {
        unsigned int statsMode;
        unsigned int leaderboardMode;
        revisionStream >> statsMode >> leaderboardMode >> mAutoShowLeaderboard;
        mSongLibStatsMode = statsMode;
        mSongLibLeaderboardMode = leaderboardMode;
    }
    if (revisionStream.mReadRevision >= 27) {
        unsigned int graphicsMode;
        unsigned int hdr;
        revisionStream >> graphicsMode >> hdr;
        mGraphicsMode = graphicsMode;
        mHDR = hdr;
    }
    if (revisionStream.mReadRevision >= 4) {
        theSongMgr->LoadSongCache(revisionStream);
        if (revisionStream.mReadRevision >= kFirstLegacySongDataRevision
            && revisionStream.mReadRevision <= kLastLegacySongDataRevision) {
            // The block follows in the underlying stream.
            RevisionStream legacyStream(stream, kLegacySongDataRevision, 0);
            if (legacyStream.BeginRead()) {
                eastl::set<int> songIds;
                eastl::map<int, Symbol> songSymbols;
                unsigned int count;
                legacyStream.ReadEndian(&count, sizeof(count));
                for (unsigned int index = 0; index < count; ++index) {
                    int songId;
                    legacyStream.ReadEndian(&songId, sizeof(songId));
                    songIds.insert(songId);
                }
                legacyStream >> songSymbols;
            }
        }
    }
    if (revisionStream.mReadRevision <= 22) {
        mIndependentTrackSpeed = true;
    }
    MakeClean();
}

// Reconstructed from eboot.elf at 0xD67570.
void RBSystemOptions::SetBackgroundVolume(unsigned int volume) {
    if (mBackgroundVolume != volume) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mBackgroundVolume = volume;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD675F0.
void RBSystemOptions::SetInstrumentsVolume(unsigned int volume) {
    if (mInstrumentsVolume != volume) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mInstrumentsVolume = volume;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67670.
void RBSystemOptions::SetSFXVolume(unsigned int volume) {
    if (mSFXVolume != volume) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mSFXVolume = volume;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD676F0.
void RBSystemOptions::SetCrowdVolume(unsigned int volume) {
    if (mCrowdVolume != volume) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mCrowdVolume = volume;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67770.
float RBSystemOptions::GetBackgroundVolumeDb() const {
    return _SliderIndexToDb(mBackgroundVolume, kMusicVolumeDb);
}

// Reconstructed from eboot.elf at 0xD67790.
float RBSystemOptions::GetInstrumentsVolumeDb() const {
    return _SliderIndexToDb(mInstrumentsVolume, kMusicVolumeDb);
}

// Reconstructed from eboot.elf at 0xD677B0.
float RBSystemOptions::GetSFXVolumeDb() const {
    return _SliderIndexToDb(mSFXVolume, kEffectsVolumeDb);
}

// Reconstructed from eboot.elf at 0xD677D0.
float RBSystemOptions::GetCrowdVolumeDb() const {
    return _SliderIndexToDb(mCrowdVolume, kEffectsVolumeDb);
}

// Reconstructed from eboot.elf at 0xD677F0.
void RBSystemOptions::SetBassBoost(bool bassBoost) {
    if (mBassBoost != bassBoost) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mBassBoost = bassBoost;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67870.
void RBSystemOptions::SetDolby(bool dolby) {
    if (mDolby != dolby) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mDolby = dolby;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD678F0.
void RBSystemOptions::SetOverscan(bool overscan) {
    if (mOverscanEnabled != overscan) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mOverscanEnabled = overscan;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67970.
void RBSystemOptions::SetIsDrumNavigationAllowed(bool allowed) {
    if (mIsDrumNavigationAllowed != allowed) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mIsDrumNavigationAllowed = allowed;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD679F0.
void RBSystemOptions::SetNoFail(bool noFail) {
    if (mNoFail != noFail) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mNoFail = noFail;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67A70.
void RBSystemOptions::SetIndependentTrackSpeed(bool independent) {
    if (mIndependentTrackSpeed != independent) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mIndependentTrackSpeed = independent;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67AF0.
void RBSystemOptions::SetProDrumCymbalLanes(unsigned int lanes) {
    if (mProDrumCymbalLanes != lanes) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mProDrumCymbalLanes = lanes;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67B70.
void RBSystemOptions::SetImprovSolosScored(bool scored) {
    if (mImprovSolosScored != scored) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mImprovSolosScored = scored;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67BF0.
void RBSystemOptions::SetSubtitles(bool subtitles) {
    if (mSubtitles != subtitles) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mSubtitles = subtitles;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67C70.
void RBSystemOptions::SetAwesomenessDetection(bool detection) {
    if (mAwesomenessDetection != detection) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mAwesomenessDetection = detection;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67CF0.
void RBSystemOptions::SetLeaderboardLadder(bool ladder) {
    if (mLeaderboardLadder != ladder) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mLeaderboardLadder = ladder;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67D70.
void RBSystemOptions::SetSongLibSort(RBSongSort::SortMethod sort) {
    if (mSongLibSort != sort) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mSongLibSort = sort;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67DF0.
void RBSystemOptions::SetHide1RatedSongs(bool hide) {
    if (mHide1RatedSongs != hide) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mHide1RatedSongs = hide;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67E70.
void RBSystemOptions::SetHideUnavailableSongs(bool hide) {
    if (mHideUnavailableSongs != hide) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mHideUnavailableSongs = hide;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67EF0.
void RBSystemOptions::SetRivalsChallengeFilter(bool filter) {
    if (mRivalsChallengeFilter != filter) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mRivalsChallengeFilter = filter;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67F70.
void RBSystemOptions::SetSongLibStatsMode(unsigned int mode) {
    if (mSongLibStatsMode != mode) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mSongLibStatsMode = mode;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD67FF0.
void RBSystemOptions::SetSongLibLeaderboardMode(unsigned int mode) {
    if (mSongLibLeaderboardMode != mode) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mSongLibLeaderboardMode = mode;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD68070.
void RBSystemOptions::SetAutoShowLeaderboard(bool show) {
    if (mAutoShowLeaderboard != show) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mAutoShowLeaderboard = show;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD680F0.
void RBSystemOptions::SetLastSeenPatch(unsigned int patch) {
    if (mLastSeenPatch != patch) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mLastSeenPatch = patch;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD68170.
void RBSystemOptions::SetHasSeenUpsell(bool seen) {
    if (mHasSeenUpsell != seen) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mHasSeenUpsell = seen;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD681F0.
void RBSystemOptions::SetAutokick(bool autokick) {
    if (mAutokick != autokick) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mAutokick = autokick;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD68270.
void RBSystemOptions::SetGraphicsMode(unsigned int mode) {
    if (mGraphicsMode != mode) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mGraphicsMode = mode;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD682F0.
void RBSystemOptions::SetHDR(unsigned int hdr) {
    if (mHDR != hdr) {
        static const long sHeap = MemFindHeap("profile");
        ProfileHeapScope scope(sHeap);
        mHDR = hdr;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD68550.
bool RBSystemOptions::GetNoFail() const {
    return mNoFail;
}

// Reconstructed from eboot.elf at 0xD68560.
bool RBSystemOptions::GetIndependentTrackSpeed() const {
    return mIndependentTrackSpeed;
}

// Reconstructed from eboot.elf at 0xD68570.
unsigned int RBSystemOptions::GetProDrumCymbalLanes() const {
    return mProDrumCymbalLanes;
}

// Reconstructed from eboot.elf at 0xD68580.
bool RBSystemOptions::GetImprovSolosScored() const {
    return mImprovSolosScored;
}

// Reconstructed from eboot.elf at 0xD68590.
bool RBSystemOptions::GetAutokick() const {
    return mAutokick;
}

// Reconstructed from eboot.elf at 0xD685A0.
float RBSystemOptions::GetAudioOffset() const {
    return mAudioOffset;
}

// Reconstructed from eboot.elf at 0xD685B0.
float RBSystemOptions::GetVideoOffset() const {
    return mVideoOffset;
}
