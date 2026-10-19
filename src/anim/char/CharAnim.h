#pragma once

#include <cstddef>

#include "anim/char/CharBoneKeyframeSet.h"
#include "anim/char/CharBoneMaskCom.h"
#include "anim/core/AnimResource.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/resources/Resource.h"
#include "entity/waveform/Key.h"
#include "utl/containers/Vector.h"
#include "utl/streams/BinStream.h"

class CharAnimBuffer;
class CharAnimRaw;
class CharSkeletonResource;

// A compressed character animation (anim/CharAnim.o, 0x32CFA0-0x331A4F):
// the frame rate, the beat at each frame, the keyframes of the bone
// channels its bone mask keeps, and how it blends. CharAnimResource holds
// one, built from a CharAnimRaw. The object is 192 bytes, without a
// vtable. Field names are not in the reference map.
class CharAnim {
public:
    // How a playback of the animation follows the clock (the map's
    // CharAnim::Units; "playback_units"). The enumerator names follow
    // UnitsToString's table ("realtime", "beat", "beatAlign1",
    // "beatAlign4", "beatAlign2", "numUnits").
    enum Units {
        // "Real-time": in seconds, the default.
        kRealTime = 0,
        // "Beat": with the beat, not aligned to it.
        kBeat = 1,
        // "Beat Aligned".
        kBeatAlign1 = 2,
        // "Downbeat Aligned".
        kBeatAlign4 = 3,
        // "Two Beat Aligned".
        kBeatAlign2 = 4,
        kNumUnits = 5,
    };

    CharAnim();   // 0x32D030
    ~CharAnim();  // 0x32D0B0

    // Allocates an empty animation.
    static CharAnim* New();  // 0x32CFA0
    // The units' name in UnitsToString's table.
    static const char* UnitsToString(Units units);  // 0x3305F0
    // The "relative" labels ("Not Relative", "Bind Relative", "Other
    // Relative") for CharAnimCom and CharClipCom. Name not in the reference
    // map.
    static eastl::vector<AllowedValue<int>> GetRelativeAllowedValues();  // 0x330600
    // The Units labels for "playback_units". Name not in the reference
    // map.
    static eastl::vector<AllowedValue<int>> GetUnitsAllowedValues();  // 0x330830

    // Reads the animation; false unless the revision is 10 and the
    // keyframes load.
    bool Load(BinStream& stream, bool cached);  // 0x32D120
    // Writes the animation at revision 10.
    void Save(BinStream& stream, bool cached) const;  // 0x32D230
    // Builds the keyframes from the raw animation with the bone mask's
    // skeleton, masks, constraints and blend settings, relative to the
    // first frame of `relative` when it is given.
    bool Create(const CharBoneMaskCom* mask, CharAnimRaw* raw, CharAnimRaw* relative, ResourcePath path);  // 0x32D450
    // The same without constraints. No caller remains. Newer than the
    // reference map's single Create.
    bool Create(
        CharSkeletonResource* skeleton,
        const PropArray<CharBoneChannelMask>& masks,
        AnimResource::BlendMode mode,
        unsigned long subTree,
        bool blendFromRoot,
        CharAnimRaw* raw,
        CharAnimRaw* relative,
        ResourcePath path);  // 0x32F980
    // The builder both Create overloads call: it samples the raw bones the
    // masks keep into a channel format, applies the constraints, removes
    // the relative pose, compresses the frames into keyframes and records
    // the root motion. Not reconstructed (9.4 KB; see
    // docs/anim-characters.md). Newer than the reference map.
    bool Create(
        CharSkeletonResource* skeleton,
        const PropArray<CharBoneChannelMask>& masks,
        const PropArray<CharBoneMaskCom::Constraint>& constraints,
        AnimResource::BlendMode mode,
        unsigned long subTree,
        bool blendFromRoot,
        CharAnimRaw* raw,
        CharAnimRaw* relative,
        ResourcePath path);  // 0x32D4B0
    // Adds the frame to the buffer through the keyframes. The map's
    // signature is Apply(float, float, float, AnimResource::BlendMode,
    // CharAnimBuffer&) const; this build adds `wrapped`.
    void Apply(
        float weight,
        float frame,
        float delta,
        bool wrapped,
        AnimResource::BlendMode mode,
        CharAnimBuffer& buffer) const;  // 0x330450
    // Scales the buffer's channels the animation drives down.
    void ScaleDown(float weight, CharAnimBuffer& buffer) const;  // 0x330460
    // The frame at a beat and the beat at a frame, through the beat track;
    // without two keys a beat is 16 frames.
    float GetFrameFromBeat(float beat) const;  // 0x330470
    float GetBeatFromFrame(float frame) const;  // 0x330540

    // The revision of the animation files: CharAnimResource and
    // CharAnimRawResource write it before their animation and load only
    // this revision.
    static const unsigned long kRev;  // 0x12752E0

    float mFramesPerSecond;
    // The beat at each frame (key frame and beat).
    TKeyTrack<float> mBeatTrack;
    CharBoneKeyframeSet mKeyframes;
    // How the animation blends (AnimResource::GetBlendMode) and its
    // blend-tree slot, from the bone mask.
    AnimResource::BlendMode mBlendMode;
    unsigned long mSubTreeIndex;
};

static_assert(offsetof(CharAnim, mBeatTrack) == 8);
static_assert(offsetof(CharAnim, mKeyframes) == 64);
static_assert(offsetof(CharAnim, mBlendMode) == 176);
static_assert(offsetof(CharAnim, mSubTreeIndex) == 184);
static_assert(sizeof(CharAnim) == 192);

// Write a float key track: revision 1, the interpolation, then the keys
// (revision 1, the count, each key's frame and value). The map has the
// writer in anim/CharAnim.o.
BinStream& operator<<(BinStream& stream, const TKeyTrack<float>& track);  // 0x32D300
// Reads the keys a track writes; revision 0 holds a count and no keys. The
// map's operator>> <TKey<float>>(BinStream&, PropArray<TKey<float>>&).
BinStream& operator>>(BinStream& stream, PropArray<TKey<float>>& keys);  // 0x331490

// Reads a float key track, upgrading the interpolation of revision 0.
// Inlined into CharAnim::Load and CharAnimRaw::Load. Name not in the
// reference map.
inline BinStream& ReadKeyTrack(BinStream& stream, TKeyTrack<float>& track) {
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    stream.ReadEndian(&track.mInterpolation, 4);
    if (rev <= 0) {
        UpgradeInterpolation(track.mInterpolation, &track.mInterpolation);
    }
    return stream >> track.mKeys;
}
