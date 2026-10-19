#pragma once

#include <cstddef>

#include "anim/skeleton/CharBoneTransform.h"
#include "entity/waveform/Key.h"
#include "math/transform/Transform.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;

// The raw samples of a character animation as exported
// (anim/CharAnimRaw.o, 0x333100-0x334DAF): the frame rate, the beat at
// each frame, and every bone's name, parent and transform at each frame.
// CharAnimRawResource holds one; CharAnim::Create compresses it into
// keyframes. The object is 128 bytes, without a vtable. Field names are
// not in the reference map.
class CharAnimRaw {
public:
    // A bone of the animation (the map's CharAnimRaw::BoneInfo; 16 bytes).
    struct BoneInfo {
        Symbol mName;
        // Whether the bone compensates its parent's scale, which an inverse
        // parent scale channel needs (CharAnim::Create clears the channel
        // otherwise). The name is weak.
        bool mInverseParentScale;
        // Not read from the file; set for the bones of the bind pose
        // CharAnimResource builds. The name is weak.
        bool mInBindPose;
        // The parent bone; a root is its own parent.
        unsigned int mParent;
    };

    // Inlined into New (0x333100).
    CharAnimRaw()
        : mFramesPerSecond(30.0f), mBeatTrack(), mSamples(), mBones() {}
    ~CharAnimRaw();  // 0x333250

    // Allocates an empty animation.
    static CharAnimRaw* New();  // 0x333100

    // Reads the animation; false unless the revision is 10. The flag is
    // not read.
    bool Load(BinStream& stream, bool cached);  // 0x333330
    // Writes the animation at revision 10.
    void Save(BinStream& stream, bool cached) const;  // 0x333670
    // Sizes the animation for the bones and frames, with the frame rate
    // and the beat track. The binary passes the frame rate in a float
    // register; the map's parameter order is kept.
    void Init(unsigned long numBones, unsigned long numFrames, float framesPerSecond, const TKeyTrack<float>& beats);  // 0x333800
    // The index of the bone with the name, or -1.
    unsigned long FindBoneIndex(Symbol name) const;  // 0x3339A0
    // Names the bone and sets its parent and flags.
    void SetBone(unsigned long bone, const char* name, unsigned long parent, bool inverseParentScale, bool inBindPose);  // 0x3339E0
    // Sets the bone's transform at the frame; true when the transform has
    // a value that is not finite (the map's CheckNaNOrInf, inlined).
    bool SetBoneSample(unsigned long bone, unsigned long frame, const CharBoneTransform& xfm);  // 0x333A60
    // The bone's transform at the frame.
    const CharBoneTransform& GetLocalTransform(unsigned long bone, unsigned long frame) const;  // 0x333C50
    // The bone's transform at the frame composed with its ancestors'.
    void GetWorldXfm(unsigned long bone, unsigned long frame, Transform& xfm) const;  // 0x333C70
    // The bone's transform at the frame relative to another bone: its own
    // local transform when that bone is itself or its parent, else the
    // relative transform of the two world transforms, orthonormalized,
    // with unit parent scale.
    void GetLocalTransformRelativeTo(
        unsigned long bone,
        unsigned long relativeTo,
        unsigned long frame,
        CharBoneTransform& xfm) const;  // 0x333F10
    // Makes the bone with the name a copy of another bone, adding it when
    // there is none.
    void CloneBone(Symbol name, unsigned long source);  // 0x3344B0

    float mFramesPerSecond;
    // The beat at each frame (key frame and beat).
    TKeyTrack<float> mBeatTrack;
    // Each bone's transform at each frame.
    eastl::vector<eastl::vector<CharBoneTransform>> mSamples;
    eastl::vector<BoneInfo> mBones;
};

static_assert(offsetof(CharAnimRaw::BoneInfo, mInverseParentScale) == 8);
static_assert(offsetof(CharAnimRaw::BoneInfo, mInBindPose) == 9);
static_assert(offsetof(CharAnimRaw::BoneInfo, mParent) == 12);
static_assert(sizeof(CharAnimRaw::BoneInfo) == 16);
static_assert(offsetof(CharAnimRaw, mBeatTrack) == 8);
static_assert(offsetof(CharAnimRaw, mSamples) == 64);
static_assert(offsetof(CharAnimRaw, mBones) == 96);
static_assert(sizeof(CharAnimRaw) == 128);
