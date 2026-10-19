#pragma once

// The instrument track types (rb_beatmatch/TrackType.o). Only the type is
// declared; the object is not reconstructed. The map names the type; the
// enumerator names and values are the allowed values of the track type
// properties (for example the one built at 0xC45160). The later
// enumerators (kTrackNone, kTrackPending) are not declared.
enum TrackType {
    kTrackDrum = 0,
    kTrackGuitar = 1,
    kTrackBass = 2,
    kTrackVocals = 3,
};
