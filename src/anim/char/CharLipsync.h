#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;

// A character's lip-sync data (0x336900-0x33A9AB, after
// anim/CharBoneStateBuffer.o; newer than the reference map, so the object
// and the names are inferred): the viseme weights of each frame, for each
// track. A frame's data is a run per track, the tracks separated by 0xFF
// bytes, each run a list of (viseme index, weight * 255) byte pairs.
// CharLipsyncResource holds one; the game's lip-sync driver plays it
// through a Playback. The object is 112 bytes.
class CharLipsync {
public:
    // Plays one track of the data into weights the game binds per viseme.
    // 48 bytes.
    class Playback {
    public:
        Playback();  // 0x336DA0

        // Plays the track of the data; the weight bindings are cleared.
        void Set(const CharLipsync* lipsync, unsigned long track);  // 0x336DE0
        // Sets the bound weights to the track's visemes at the time, blended
        // between the two frames around it; visemes the frames do not name
        // get 0. Nothing happens without data or with fewer than two
        // frames.
        void Poll(float time);  // 0x336E80
        // The start of the track's run in the frame's data, with the frame's
        // end in `end`; the end or later when the frame has no such track.
        // No caller remains; Poll inlines it.
        unsigned long FindTrackData(unsigned long frame, unsigned long& end) const;  // 0x337050

        // A weight for each viseme, or null where none is bound.
        eastl::vector<float*> mWeights;
        const CharLipsync* mLipsync;
        unsigned long mTrack;
    };

    CharLipsync();   // 0x336900
    ~CharLipsync();  // 0x336980

    // Reads the data; false unless the revision is 0.
    bool Load(BinStream& stream);  // 0x336A00
    // Writes the data at revision 0.
    void Save(BinStream& stream) const;  // 0x336BE0
    // Builds the data from a FaceFX lip-sync XML file through its
    // "FxBonePoseNode" and "FxCombinerNode" ("sum", "multiply") node graph;
    // null when the file cannot be read. Not reconstructed (0x337520-
    // 0x33A9AB, about 13 KB).
    static CharLipsync* LoadFromXml(ResourcePath path);  // 0x337520

    float mFramesPerSecond;
    // The visemes the weight pairs index.
    eastl::vector<Symbol> mVisemes;
    // The tracks of each frame. The name is weak.
    eastl::vector<Symbol> mTracks;
    // Where each frame's data starts in mData, then the data's size; frame
    // k is the bytes from offset k to offset k + 1.
    eastl::vector<unsigned int> mFrameOffsets;
    // Allocated with new[] and freed with a scalar delete, as the binary
    // does.
    unsigned char* mData;
};

static_assert(offsetof(CharLipsync::Playback, mLipsync) == 32);
static_assert(offsetof(CharLipsync::Playback, mTrack) == 40);
static_assert(sizeof(CharLipsync::Playback) == 48);
static_assert(offsetof(CharLipsync, mVisemes) == 8);
static_assert(offsetof(CharLipsync, mTracks) == 40);
static_assert(offsetof(CharLipsync, mFrameOffsets) == 72);
static_assert(offsetof(CharLipsync, mData) == 104);
static_assert(sizeof(CharLipsync) == 112);
