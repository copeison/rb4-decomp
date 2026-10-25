#include "rb_game/phrases/CommonPhraseCapturer.h"

#include "rb_beatmatch/gems/GameGem.h"
#include "rb_game/game/Game.h"
#include "rb_game/players/RBGemPlayer.h"
#include "rb_game/song/SongDB.h"
#include "rb_game/users/BandUser.h"
#include "utl/messages/Message.h"
#include "utl/text/Symbol.h"

namespace {

// The player state that keeps a track out of the common phrases. Name not
// in the reference map.
constexpr int kPlayerStateExcluded = 4;

// Whether the mask holds more than one track. Name not in the reference
// map.
bool HasSeveralTracks(int tracks) {
    return (tracks & (tracks - 1)) != 0;
}

// A script node holding the player. Name not in the reference map.
DataNode PlayerNode(RBPlayer* player) {
    DataNode node;
    node.mType = kDataObject;
    node.mValue.object = player;
    return node;
}

}  // namespace

// Reconstructed from eboot.elf at 0xA41BC0. Reset is inlined.
CommonPhraseCapturer::CommonPhraseCapturer()
    : mDisabledTracks(0), mEndedTracks(0), mUnisonPhrase(-1), mInUnisonPhrase(false), mMultiplayerUnison(false) {
    ExtendPhraseStates(50);
    Reset();
}

// Reconstructed from eboot.elf at 0xA41CD0.
void CommonPhraseCapturer::ExtendPhraseStates(int phrase) {
    if (mPhraseStates.size() > static_cast<unsigned long>(phrase)) {
        return;
    }
    mPhraseStates.resize(phrase + 1);
}

// Reconstructed from eboot.elf at 0xA41D20.
void CommonPhraseCapturer::Reset() {
    mDisabledTracks = 0;
    mUnisonPhrase = -1;
    for (RBPlayer* const player : theGame->GetActivePlayers()) {
        if (player->mPlayerState == kPlayerStateExcluded) {
            mDisabledTracks |= 1 << player->mTrack;
        }
    }
    mEndedTracks = 0;
    mInUnisonPhrase = false;
    mMultiplayerUnison = false;
    for (PhraseState& state : mPhraseStates) {
        state = PhraseState();
    }
}

// Reconstructed from eboot.elf at 0xA42240.
int CommonPhraseCapturer::ComputeTracks(int phrase) const {
    return theSongDB->GetCommonPhraseTracks(phrase) & ~mDisabledTracks;
}

// Reconstructed from eboot.elf at 0xA42270.
bool CommonPhraseCapturer::IsUnisonPhrase(int phrase) const {
    return theSongDB->IsUnisonPhrase(phrase);
}

// Reconstructed from eboot.elf at 0xA42280. The message type is built on
// each export.
void CommonPhraseCapturer::_CheckUnisonPhraseCompleted(int phrase, int tracks) {
    if ((tracks & ~mEndedTracks) != 0) {
        return;
    }
    const int shared = theSongDB->GetCommonPhraseTracks(phrase) | mPhraseStates[phrase].mFailedTracks;
    if (HasSeveralTracks(shared) || mMultiplayerUnison) {
        Message<0> msg(Symbol("multiplayer_unison_phrase_end"));
        theMutableGame->Export(msg.mData);
    }
    mEndedTracks = 0;
    mInUnisonPhrase = false;
    mMultiplayerUnison = false;
}

// Reconstructed from eboot.elf at 0xA42340.
bool CommonPhraseCapturer::IsMultiplayerPhrase(int phrase) const {
    const int tracks = theSongDB->GetCommonPhraseTracks(phrase) | mPhraseStates[phrase].mFailedTracks;
    return HasSeveralTracks(tracks);
}

// Reconstructed from eboot.elf at 0xA42380. The search runs to the
// phrase's last gem, then back over every gem of the phrase.
bool CommonPhraseCapturer::_HasPlayedWholePhrase(const RBGemPlayer& player, int phrase, unsigned long gem) const {
    const int track = player.mTrack;
    if ((mPhraseStates[phrase].mFailedTracks & (1 << track)) != 0) {
        return false;
    }
    const eastl::vector<GameGem>& gems = theSongDB->GetGems(track);
    unsigned long end = gem + 1;
    while (end < gems.size() && theSongDB->GetPhraseID(track, end) == phrase) {
        end++;
    }
    for (unsigned long i = end; i-- != 0;) {
        if (theSongDB->GetPhraseID(track, i) != phrase) {
            return true;
        }
        if (gems[i].PlayableBy(player.mUser->GetSlot()) && !player.HasHitGem(i)) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xA42730. OneTrackCompletedPhrase is
// inlined; the message type is built on each export.
void CommonPhraseCapturer::LocalHitLastGem(int phrase, int track) {
    RBPlayer* const player = theMutableGame->GetPlayerFromTrack(track, true);
    Message<2> msg(Symbol("phrase_track_success"));
    msg.mData->Node(2) = PlayerNode(player);
    msg.mData->Node(3) = DataNode(phrase);
    player->Export(msg.mData);
    OneTrackCompletedPhrase(phrase, track);
    if (!theSongDB->IsUnisonPhrase(phrase)) {
        return;
    }
    ExtendPhraseStates(phrase);
    if (mPhraseStates[phrase].mStatus == kPhraseFailed) {
        return;
    }
    mPhraseStates[phrase].mHitTracks |= 1 << track;
    const int tracks = theSongDB->GetCommonPhraseTracks(phrase) | mPhraseStates[phrase].mFailedTracks;
    if ((tracks & ~(mDisabledTracks | mPhraseStates[phrase].mHitTracks)) == 0) {
        AllTracksCompletedPhrase(phrase);
    }
}

// Reconstructed from eboot.elf at 0xA42A60.
void CommonPhraseCapturer::OneTrackCompletedPhrase(int phrase, int track) {
    static_cast<void>(phrase);
    theMutableGame->GetPlayerFromTrack(track, true)->EnergyPhraseHit();
}

// Reconstructed from eboot.elf at 0xA42A90. ComputeTracks is inlined; the
// message type is built on each export.
void CommonPhraseCapturer::AllTracksCompletedPhrase(int phrase) {
    const int tracks = ComputeTracks(phrase);
    const bool shared = tracks != 0 && HasSeveralTracks(tracks);
    for (unsigned long i = 0; i < theSongDB->GetNumTracks(); i++) {
        if ((tracks & (1 << i)) == 0) {
            continue;
        }
        RBPlayer* const player = theMutableGame->GetPlayerFromTrack(static_cast<int>(i), true);
        if (shared) {
            player->BandUnisonPhraseHit();
        } else {
            player->SoloUnisonPhraseHit();
        }
    }
    Message<0> msg(Symbol("unison_phrase_success"));
    theMutableGame->Export(msg.mData);
    ExtendPhraseStates(phrase);
    mPhraseStates[phrase].mStatus = kPhraseSucceeded;
}

// Reconstructed from eboot.elf at 0xA42C50. ComputeTracks is inlined; the
// message type is built on each export.
void CommonPhraseCapturer::LocalFail(RBPlayer& player, int phrase, int track) {
    const int tracks = ComputeTracks(phrase);
    if (tracks != 0) {
        player.EnergyPhraseMiss(phrase);
    }
    ExtendPhraseStates(phrase);
    mPhraseStates[phrase].mStatus = kPhraseFailed;
    mPhraseStates[phrase].mFailedTracks |= 1 << track;
    if (!theSongDB->IsUnisonPhrase(phrase)) {
        return;
    }
    Message<0> msg(Symbol("unison_phrase_failure"));
    theMutableGame->Export(msg.mData);
    const int shared = theSongDB->GetCommonPhraseTracks(phrase) | mPhraseStates[phrase].mFailedTracks;
    if (!HasSeveralTracks(shared)) {
        return;
    }
    for (unsigned long i = 0; i < theSongDB->GetNumTracks(); i++) {
        if ((tracks & (1 << i)) != 0) {
            theMutableGame->GetPlayerFromTrack(static_cast<int>(i), true)->UnisonPhraseMiss(phrase);
        }
    }
}

// Reconstructed from eboot.elf at 0xA42E00.
int CommonPhraseCapturer::GetPhraseStatus(int phrase) const {
    if (phrase == -1 || static_cast<unsigned long>(phrase) >= mPhraseStates.size()) {
        return kPhraseUndecided;
    }
    return mPhraseStates[phrase].mStatus;
}

// Reconstructed from eboot.elf at 0xA42E40.
bool CommonPhraseCapturer::DidTrackFail(int phrase, int track) const {
    if (phrase == -1 || static_cast<unsigned long>(phrase) >= mPhraseStates.size()) {
        return false;
    }
    return (mPhraseStates[phrase].mFailedTracks & (1 << track)) != 0;
}
