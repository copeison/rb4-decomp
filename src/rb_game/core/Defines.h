#pragma once

#include "entity/props/PropMetadata.h"
#include "rb_beatmatch/tracks/TrackType.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class Entity;

// The game's shared definitions (rb_game/Defines.o, 0xA43F20-0xA459E8):
// the controller, score type and band slot conversions and the cheated
// score reasons. Two predicates at its start (0xA43EF0, 0xA43F10) classify
// a game session value the reconstruction has not identified and are not
// reconstructed. The map's CensorString and WinMetricToSym are not in this
// build's object.

// The four player slots of a band. The map names the type (for example
// RBMetaState::IsSlotJoined(GameSlot)); the enumerator names are the
// strings of GameSlotToString's table. kNumGameSlots also stands for "no
// particular slot".
enum GameSlot {
    kGameSlotGuitar0 = 0,
    kGameSlotDrum = 1,
    kGameSlotVocals = 2,
    kGameSlotGuitar1 = 3,
    kNumGameSlots = 4,
};

static_assert(sizeof(GameSlot) == 4);

// The kind of instrument controller, in the order of the
// CHAR_INSTRUMENT_SYMBOLS macro. The map names the type; the enumerator
// names are not in the reference map.
enum ControllerType {
    kControllerDrum = 0,
    kControllerGuitar = 1,
    kControllerVocals = 2,
    kControllerKeys = 3,
    kControllerNone = 4,
};

// The score a player's part counts toward. The map names the type; the
// enumerator names are the texts of GetAllowedScoreTypeValues.
enum ScoreType {
    kScoreGuitar = 0,
    kScoreBass = 1,
    kScoreDrum = 2,
    kScoreRealDrum = 3,
    kScoreVocals = 4,
    kScoreHarmony = 5,
    kScoreBand = 6,
    kAllScores = 7,
};

// Why a score was marked cheated. Newer than the map; the type and the
// enumerator names are not in the reference map and follow the texts of
// GetAllowedCheatedReasonValues.
enum CheatedReason {
    kCheatedReasonNone = -1,
    kCheatedReasonAutoKick = 0,
    kCheatedReasonODDrainSqueeze = 1,
};

// The controller each slot is played with; kControllerNone for an invalid
// slot. The map's GameSlotToControllerType(GameSlot).
ControllerType GameSlotToControllerType(GameSlot slot);  // 0xA43F20
// The controller that plays the part; kControllerNone for other tracks.
ControllerType TrackTypeToControllerType(TrackType type);  // 0xA43F40
// The part a controller plays. A guitar plays bass and a keyboard the
// second keyboard part when `alternate` is set. The map has
// ControllerTypeToTrackType(ControllerType, bool).
TrackType ControllerTypeToTrackType(ControllerType type, bool alternate);  // 0xA43F60
// The part a slot plays; the second guitar slot plays bass when `bass` is
// set. The map has GameSlotToTrackType(GameSlot); this build adds the flag.
TrackType GameSlotToTrackType(GameSlot slot, bool bass);  // 0xA43FA0
// The slot's enumerator name, or "INVALID_GAME_SLOT". Name not in the
// reference map.
const char* GameSlotToString(GameSlot slot);  // 0xA43FE0
// The score a part counts toward, the harmony and pro drum scores when
// asked for; kScoreBand for the band tracks and kAllScores for the rest.
ScoreType TrackTypeToScoreType(TrackType type, bool harmony, bool realDrums);  // 0xA44000
// The score type with the pro drum and harmony scores folded into the drum
// and vocal scores. Name not in the reference map.
ScoreType GetBaseScoreType(ScoreType type);  // 0xA44040
// The type's CHAR_INSTRUMENT_SYMBOLS entry, or "unknown".
Symbol ControllerTypeToSym(ControllerType type);  // 0xA44060
ControllerType SymToControllerType(Symbol sym);  // 0xA44130
// The type's SCORE_TYPE_SYMBOLS entry, or "unknown".
Symbol ScoreTypeToSym(ScoreType type);  // 0xA441A0
// The type whose symbol matches, else kAllScores.
ScoreType SymToScoreType(Symbol sym);  // 0xA44270
// The part a score is played on; kTrackNone for the band score.
TrackType ScoreTypeToTrackType(ScoreType type);  // 0xA44310
// The score of the only band user's part, or the band score when there are
// several users. Not reconstructed: it calls BandUser virtuals that are not
// modelled yet.
ScoreType GetActiveScoreType();  // 0xA44330
// The controller a score is played on, or kControllerNone. Name not in
// the reference map.
ControllerType ScoreTypeToControllerType(ScoreType type);  // 0xA44440
// The statistics symbol of the score type: ScoreTypeToSym's, and
// "stats_player_total" for kAllScores. Name not in the reference map.
Symbol ScoreTypeToStatsSym(ScoreType type);  // 0xA44460
// The allowed values of a score type property, with kAllScores when
// `includeAll` is set.
eastl::vector<AllowedValue<int>> GetAllowedScoreTypeValues(bool includeAll);  // 0xA44510
// Appends the parts the controller can play: a guitar plays bass and
// guitar, a keyboard both keyboard parts as well.
void GetTracksPlayableByController(ControllerType type, eastl::vector<TrackType>& tracks);  // 0xA44A00
bool IsTrackPlayableByController(TrackType track, ControllerType type);  // 0xA44F40
// Appends the parts that stand for the same part as the track: the track
// itself, or both keyboard parts.
void GetTracksRepresentativeOfPart(TrackType track, eastl::vector<TrackType>& tracks);  // 0xA45000
bool RepresentSamePart(TrackType track, TrackType other);  // 0xA453C0
bool IsRepresentativePartPlayableByController(TrackType track, ControllerType type);  // 0xA45480
// Whether the controller is the one the part's players prefer.
bool ControllerHasRepresentativePartPriority(ControllerType type, TrackType track);  // 0xA455E0
// The reason's text, or "UNKNOWN CHEATED REASON". Name not in the
// reference map.
const char* CheatedReasonToString(CheatedReason reason);  // 0xA45620
// The reason's localization token. Name not in the reference map.
Symbol CheatedReasonToToken(CheatedReason reason);  // 0xA45640
// Whether the reason is the automatic kick drum. Name not in the reference
// map.
bool IsAutoKickCheatedReason(CheatedReason reason);  // 0xA457B0
// The allowed values of a cheated reason property. Name not in the
// reference map.
eastl::vector<AllowedValue<int>> GetAllowedCheatedReasonValues();  // 0xA457C0

// The entity itself or the nearest entity out through the instancing
// parent objects whose resource is a UILayoutResource, or null. Newer than
// the map; the name is inferred. It sits with the city, venue and band
// name tokens (0x9A1EB0-0x9A3E10), not in this object. Not reconstructed.
Entity* GetUILayoutEntity(Entity* entity);  // 0x9A39E0
