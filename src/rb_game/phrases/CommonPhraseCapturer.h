#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"

class RBGemPlayer;
class RBPlayer;

// Follows the band's common phrases (the overdrive and unison phrases that
// several tracks share) through the song (rb_game/CommonPhraseCapturer.o,
// 0xA41BC0-0xA43090): which tracks hit each phrase's last gem and which
// failed it, and the unison phrases' begin, end, success and failure
// messages. The band keeps one (Band +0xC8); the players feed it their
// phrase gems, and the game applies the remote players' hits and failures
// to it through LocalHitLastGem and LocalFail. Not polymorphic; the object
// is 48 bytes. The binary keeps the vector's insertion
// (eastl::vector<PhraseState>::DoInsertValues, the map's name) at 0xA42F80.
//
// The members that go through the player's controller (RBPlayer +0x4F8:
// HandlePhraseNote, HitLastGem, Fail, HandleVocalPhrase, Enabled) are
// declared only: the controller's class is not modelled.
class CommonPhraseCapturer {
public:
    // A phrase's outcome. Names not in the reference map.
    enum PhraseStatus {
        kPhraseUndecided = 0,
        kPhraseSucceeded = 1,
        kPhraseFailed = 2,
    };

    // One common phrase: its outcome and the tracks that hit its last gem
    // and that failed it, as bit masks. 12 bytes; the map names the type.
    // Field names are not in the reference map.
    struct PhraseState {
        PhraseState() : mStatus(kPhraseUndecided), mHitTracks(0), mFailedTracks(0) {}

        int mStatus;
        int mHitTracks;
        int mFailedTracks;
    };

    // Room for 51 phrases, then Reset.
    CommonPhraseCapturer();  // 0xA41BC0
    // Makes room for the phrase.
    void ExtendPhraseStates(int phrase);  // 0xA41CD0
    // Forgets every phrase's outcome; the tracks of the players in state 4
    // take no part.
    void Reset();  // 0xA41D20
    // A gem of a common phrase was hit or missed: the phrase's last gem
    // ends the track's part of a unison phrase, its first gem begins it
    // (exporting "multiplayer_unison_phrase_begin" when several tracks
    // share it), a hit of the whole phrase reaches HitLastGem and a miss
    // Fail. Not reconstructed: it goes through the player's controller.
    void HandlePhraseNote(RBGemPlayer& player, unsigned long gem, bool hit);  // 0xA41DE0
    // The tracks that share the phrase and take part.
    int ComputeTracks(int phrase) const;  // 0xA42240
    bool IsUnisonPhrase(int phrase) const;  // 0xA42270
    // Ends the unison phrase once every taking part track ended it,
    // exporting "multiplayer_unison_phrase_end" when it was shared.
    void _CheckUnisonPhraseCompleted(int phrase, int tracks);  // 0xA42280
    // Whether more than one track shares or failed the phrase.
    bool IsMultiplayerPhrase(int phrase) const;  // 0xA42340
    // Whether the player hit every gem of the phrase it plays, up to the
    // phrase's end; a track that failed the phrase did not.
    bool _HasPlayedWholePhrase(const RBGemPlayer& player, int phrase, unsigned long gem) const;  // 0xA42380
    // The track hit the phrase's last gem: applied here when the track's
    // player is local, and sent through its controller. Not reconstructed.
    void HitLastGem(int phrase, int track);  // 0xA424D0
    // The player failed the phrase, likewise. Not reconstructed.
    void Fail(RBPlayer& player, int phrase);  // 0xA42550
    // A vocal phrase ended, hit or failed, unless decided. Not
    // reconstructed.
    void HandleVocalPhrase(RBPlayer& player, int phrase, bool hit);  // 0xA425D0
    // Exports "phrase_track_success" from the track's player, scores its
    // energy phrase and, in a unison phrase, records the track; the last
    // track completes the phrase.
    void LocalHitLastGem(int phrase, int track);  // 0xA42730
    // The track's player hit an energy phrase.
    void OneTrackCompletedPhrase(int phrase, int track);  // 0xA42A60
    // Every track hit the unison phrase: each track's player scores it (as
    // the band's when several tracks share it), "unison_phrase_success" is
    // exported and the phrase succeeds.
    void AllTracksCompletedPhrase(int phrase);  // 0xA42A90
    // The player missed its energy phrase; the phrase fails for the track
    // and, in a unison phrase, "unison_phrase_failure" is exported and the
    // other players lose it too.
    void LocalFail(RBPlayer& player, int phrase, int track);  // 0xA42C50
    // The phrase's PhraseStatus; undecided for an unknown phrase.
    int GetPhraseStatus(int phrase) const;  // 0xA42E00
    bool DidTrackFail(int phrase, int track) const;  // 0xA42E40
    // Lets the player's track take part again, or takes it out, failing
    // the common phrase it is in at the tick. Not reconstructed: the
    // failure goes through the player's controller.
    void Enabled(RBPlayer& player, int tick, bool enabled);  // 0xA42E80

    // Field names are not in the reference map.
    eastl::vector<PhraseState> mPhraseStates;
    // The tracks that take no part, as a bit mask.
    int mDisabledTracks;
    // The tracks that reached the current unison phrase's end.
    int mEndedTracks;
    // The current unison phrase, or -1.
    int mUnisonPhrase;
    bool mInUnisonPhrase;
    // Whether the current unison phrase began as a shared one.
    bool mMultiplayerUnison;
};

static_assert(sizeof(CommonPhraseCapturer::PhraseState) == 12);
static_assert(offsetof(CommonPhraseCapturer, mDisabledTracks) == 32);
static_assert(offsetof(CommonPhraseCapturer, mUnisonPhrase) == 40);
static_assert(offsetof(CommonPhraseCapturer, mMultiplayerUnison) == 45);
static_assert(sizeof(CommonPhraseCapturer) == 48);
