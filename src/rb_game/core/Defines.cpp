#include "rb_game/core/Defines.h"

#include "rb_game/users/BandUser.h"
#include "rb_game/users/BandUserMgr.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataUtl.h"
#include "utl/text/Str.h"

namespace {

// Whether the part list holds the track. Inlined into the playable-part
// queries. Name not in the reference map.
bool ContainsTrack(const eastl::vector<TrackType>& tracks, TrackType track) {
    const TrackType* it = tracks.begin();
    while (it != tracks.end() && *it != track) {
        ++it;
    }
    return it != tracks.end();
}

}  // namespace

// Reconstructed from eboot.elf at 0xA43F20.
ControllerType GameSlotToControllerType(GameSlot slot) {
    static const ControllerType kSlotControllers[kNumGameSlots] = {
        kControllerGuitar, kControllerDrum, kControllerVocals, kControllerGuitar};
    if (static_cast<unsigned int>(slot) < kNumGameSlots) {
        return kSlotControllers[slot];
    }
    return kControllerNone;
}

// Reconstructed from eboot.elf at 0xA43F40.
ControllerType TrackTypeToControllerType(TrackType type) {
    static const ControllerType kTrackControllers[] = {
        kControllerDrum, kControllerGuitar, kControllerGuitar, kControllerVocals, kControllerKeys, kControllerKeys};
    if (static_cast<unsigned int>(type) <= kTrackRealKeys) {
        return kTrackControllers[type];
    }
    return kControllerNone;
}

// Reconstructed from eboot.elf at 0xA43F60.
TrackType ControllerTypeToTrackType(ControllerType type, bool alternate) {
    switch (type) {
    case kControllerDrum:
        return kTrackDrum;
    case kControllerGuitar:
        return alternate ? kTrackBass : kTrackGuitar;
    case kControllerVocals:
        return kTrackVocals;
    case kControllerKeys:
        return alternate ? kTrackRealKeys : kTrackKeys;
    default:
        return kTrackNone;
    }
}

// Reconstructed from eboot.elf at 0xA43FA0.
TrackType GameSlotToTrackType(GameSlot slot, bool bass) {
    switch (slot) {
    case kGameSlotGuitar0:
        return kTrackGuitar;
    case kGameSlotDrum:
        return kTrackDrum;
    case kGameSlotVocals:
        return kTrackVocals;
    case kGameSlotGuitar1:
        return bass ? kTrackBass : kTrackGuitar;
    default:
        return kTrackNone;
    }
}

// Reconstructed from eboot.elf at 0xA43FE0.
const char* GameSlotToString(GameSlot slot) {
    static const char* const kSlotNames[kNumGameSlots] = {
        "kGameSlotGuitar0", "kGameSlotDrum", "kGameSlotVocals", "kGameSlotGuitar1"};
    if (static_cast<unsigned int>(slot) <= kGameSlotGuitar1) {
        return kSlotNames[slot];
    }
    return "INVALID_GAME_SLOT";
}

// Reconstructed from eboot.elf at 0xA44000.
ScoreType TrackTypeToScoreType(TrackType type, bool harmony, bool realDrums) {
    switch (static_cast<int>(type)) {
    case kTrackDrum:
        return realDrums ? kScoreRealDrum : kScoreDrum;
    case kTrackGuitar:
        return kScoreGuitar;
    case kTrackBass:
        return kScoreBass;
    case kTrackVocals:
        return harmony ? kScoreHarmony : kScoreVocals;
    case kTrackKeys:
    case kTrackNone:
    case 7:
    case 8:
        return kScoreBand;
    default:
        return kAllScores;
    }
}

// Reconstructed from eboot.elf at 0xA44040.
ScoreType GetBaseScoreType(ScoreType type) {
    if (type == kScoreRealDrum) {
        return kScoreDrum;
    }
    if (type == kScoreHarmony) {
        return kScoreVocals;
    }
    return type;
}

// Reconstructed from eboot.elf at 0xA44060.
Symbol ControllerTypeToSym(ControllerType type) {
    static Symbol unknown;
    if (unknown == Symbol()) {
        unknown = Symbol("unknown");
    }
    if (static_cast<unsigned int>(type) <= kControllerNone) {
        return DataGetMacro(Symbol("CHAR_INSTRUMENT_SYMBOLS"))->Sym(type);
    }
    return unknown;
}

// Reconstructed from eboot.elf at 0xA44130.
ControllerType SymToControllerType(Symbol sym) {
    for (int type = kControllerDrum; type <= kControllerNone; ++type) {
        if (ControllerTypeToSym(static_cast<ControllerType>(type)) == sym) {
            return static_cast<ControllerType>(type);
        }
    }
    return kControllerNone;
}

// Reconstructed from eboot.elf at 0xA441A0.
Symbol ScoreTypeToSym(ScoreType type) {
    static Symbol unknown;
    if (unknown == Symbol()) {
        unknown = Symbol("unknown");
    }
    if (static_cast<unsigned int>(type) <= kAllScores) {
        return DataGetMacro(Symbol("SCORE_TYPE_SYMBOLS"))->Sym(type);
    }
    return unknown;
}

// Reconstructed from eboot.elf at 0xA44270.
ScoreType SymToScoreType(Symbol sym) {
    for (int type = kScoreGuitar; type < kAllScores; ++type) {
        if (ScoreTypeToSym(static_cast<ScoreType>(type)) == sym) {
            return static_cast<ScoreType>(type);
        }
    }
    return kAllScores;
}

// Reconstructed from eboot.elf at 0xA44310.
TrackType ScoreTypeToTrackType(ScoreType type) {
    static const TrackType kScoreTracks[kAllScores] = {
        kTrackGuitar, kTrackBass, kTrackDrum, kTrackDrum, kTrackVocals, kTrackVocals, kTrackNone};
    if (static_cast<unsigned int>(type) < kAllScores) {
        return kScoreTracks[type];
    }
    return kTrackNone;
}

// Reconstructed from eboot.elf at 0xA44330. A user without a part counts
// toward the score of the controller's part. The release body reads the
// first user even when no user takes part.
ScoreType GetActiveScoreType() {
    const eastl::vector<BandUser*> users = theBandUserMgr->GetBandUsers();
    if (users.size() >= 2) {
        return kScoreBand;
    }
    const BandUser* const user = users[0];
    if (user->GetTrackType() == kTrackNone) {
        switch (user->GetControllerType()) {
        case kControllerDrum:
            return user->GetPreferredScoreType() == kScoreRealDrum ? kScoreRealDrum : kScoreDrum;
        case kControllerGuitar:
            return kScoreGuitar;
        case kControllerVocals:
            return user->GetPreferredScoreType() == kScoreHarmony ? kScoreHarmony : kScoreVocals;
        default:
            return kScoreBand;
        }
    }
    switch (static_cast<unsigned int>(user->GetTrackType())) {
    case kTrackDrum:
        return user->GetPreferredScoreType() == kScoreRealDrum ? kScoreRealDrum : kScoreDrum;
    case kTrackGuitar:
        return kScoreGuitar;
    case kTrackBass:
        return kScoreBass;
    case kTrackVocals:
        return user->GetPreferredScoreType() == kScoreHarmony ? kScoreHarmony : kScoreVocals;
    case kTrackKeys:
    case kTrackNone:
    case 7:
    case 8:
        return kScoreBand;
    default:
        return kAllScores;
    }
}

// Reconstructed from eboot.elf at 0xA44440.
ControllerType ScoreTypeToControllerType(ScoreType type) {
    static const ControllerType kScoreControllers[] = {
        kControllerGuitar, kControllerGuitar, kControllerDrum, kControllerDrum, kControllerVocals, kControllerVocals};
    if (static_cast<unsigned int>(type) <= kScoreHarmony) {
        return kScoreControllers[type];
    }
    return kControllerNone;
}

// Reconstructed from eboot.elf at 0xA44460.
Symbol ScoreTypeToStatsSym(ScoreType type) {
    if (type != kAllScores) {
        return ScoreTypeToSym(type);
    }
    static Symbol statsPlayerTotal;
    if (statsPlayerTotal == Symbol()) {
        statsPlayerTotal = Symbol("stats_player_total");
    }
    return statsPlayerTotal;
}

// Reconstructed from eboot.elf at 0xA44510.
eastl::vector<AllowedValue<int>> GetAllowedScoreTypeValues(bool includeAll) {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(7);
    values.push_back(AllowedValue<int>{kScoreGuitar, String("kScoreGuitar"), String("")});
    values.push_back(AllowedValue<int>{kScoreBass, String("kScoreBass"), String("")});
    values.push_back(AllowedValue<int>{kScoreDrum, String("kScoreDrum"), String("")});
    values.push_back(AllowedValue<int>{kScoreRealDrum, String("kScoreRealDrum"), String("")});
    values.push_back(AllowedValue<int>{kScoreVocals, String("kScoreVocals"), String("")});
    values.push_back(AllowedValue<int>{kScoreHarmony, String("kScoreHarmony"), String("")});
    values.push_back(AllowedValue<int>{kScoreBand, String("kScoreBand"), String("")});
    if (includeAll) {
        values.push_back(AllowedValue<int>{kAllScores, String("kAllScores"), String("")});
    }
    return values;
}

// Reconstructed from eboot.elf at 0xA44A00.
void GetTracksPlayableByController(ControllerType type, eastl::vector<TrackType>& tracks) {
    switch (type) {
    case kControllerDrum:
        tracks.push_back(kTrackDrum);
        break;
    case kControllerGuitar:
        tracks.push_back(kTrackBass);
        tracks.push_back(kTrackGuitar);
        break;
    case kControllerVocals:
        tracks.push_back(kTrackVocals);
        break;
    case kControllerKeys:
        tracks.push_back(kTrackBass);
        tracks.push_back(kTrackGuitar);
        tracks.push_back(kTrackKeys);
        tracks.push_back(kTrackRealKeys);
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0xA44F40.
bool IsTrackPlayableByController(TrackType track, ControllerType type) {
    eastl::vector<TrackType> tracks;
    GetTracksPlayableByController(type, tracks);
    return ContainsTrack(tracks, track);
}

// Reconstructed from eboot.elf at 0xA45000.
void GetTracksRepresentativeOfPart(TrackType track, eastl::vector<TrackType>& tracks) {
    switch (track) {
    case kTrackDrum:
        tracks.push_back(kTrackDrum);
        break;
    case kTrackGuitar:
        tracks.push_back(kTrackGuitar);
        break;
    case kTrackBass:
        tracks.push_back(kTrackBass);
        break;
    case kTrackVocals:
        tracks.push_back(kTrackVocals);
        break;
    case kTrackKeys:
    case kTrackRealKeys:
        tracks.push_back(kTrackKeys);
        tracks.push_back(kTrackRealKeys);
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0xA453C0.
bool RepresentSamePart(TrackType track, TrackType other) {
    eastl::vector<TrackType> tracks;
    GetTracksRepresentativeOfPart(track, tracks);
    return ContainsTrack(tracks, other);
}

// Reconstructed from eboot.elf at 0xA45480.
bool IsRepresentativePartPlayableByController(TrackType track, ControllerType type) {
    eastl::vector<TrackType> parts;
    GetTracksRepresentativeOfPart(track, parts);
    for (unsigned long i = 0; i < parts.size(); ++i) {
        const TrackType part = parts[i];
        eastl::vector<TrackType> playable;
        GetTracksPlayableByController(type, playable);
        if (ContainsTrack(playable, part)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xA455E0.
bool ControllerHasRepresentativePartPriority(ControllerType type, TrackType track) {
    switch (type) {
    case kControllerDrum:
        return track == kTrackDrum;
    case kControllerGuitar:
        return track == kTrackGuitar || track == kTrackBass;
    case kControllerVocals:
        return track == kTrackVocals;
    case kControllerKeys:
        return track == kTrackKeys || track == kTrackRealKeys;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0xA45620.
const char* CheatedReasonToString(CheatedReason reason) {
    static const char* const kReasonNames[] = {"Invalid/None", "Auto-Kick", "OD Drain Squeeze"};
    const unsigned int index = reason + 1;
    if (index > 2) {
        return "UNKNOWN CHEATED REASON";
    }
    return kReasonNames[index];
}

// Reconstructed from eboot.elf at 0xA45640. A reason out of range keeps the
// last token looked up, the unknown reason's.
Symbol CheatedReasonToToken(CheatedReason reason) {
    static Symbol autokickScore;
    if (autokickScore == Symbol()) {
        autokickScore = Symbol("autokick_score");
    }
    static Symbol odDrainScore;
    if (odDrainScore == Symbol()) {
        odDrainScore = Symbol("od_drain_score_NEEDS_TOKEN");
    }
    static Symbol unknownScoreReason;
    if (unknownScoreReason == Symbol()) {
        unknownScoreReason = Symbol("unknown_score_reason_NEEDS_TOKEN");
    }
    static Symbol* const kTokens[] = {&unknownScoreReason, &autokickScore, &odDrainScore};
    const unsigned int index = reason + 1;
    if (index > 2) {
        return unknownScoreReason;
    }
    return *kTokens[index];
}

// Reconstructed from eboot.elf at 0xA457B0.
bool IsAutoKickCheatedReason(CheatedReason reason) {
    return reason == kCheatedReasonAutoKick;
}

// Reconstructed from eboot.elf at 0xA457C0. Each value's description repeats
// its name.
eastl::vector<AllowedValue<int>> GetAllowedCheatedReasonValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(2);
    values.push_back(AllowedValue<int>{kCheatedReasonNone, String("Invalid/None"), String("Invalid/None")});
    values.push_back(AllowedValue<int>{kCheatedReasonAutoKick, String("Auto-Kick"), String("Auto-Kick")});
    values.push_back(
        AllowedValue<int>{kCheatedReasonODDrainSqueeze, String("OD Drain Squeeze"), String("OD Drain Squeeze")});
    return values;
}
