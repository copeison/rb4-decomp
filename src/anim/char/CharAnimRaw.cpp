// anim/CharAnimRaw.o (0x333100 to 0x334DAF). The vector growth templates
// at 0x334790-0x334DA0 are the containers'.
#include "anim/char/CharAnimRaw.h"

#include <math.h>

#include "anim/char/CharAnim.h"
#include "math/rotation/Quat.h"
#include "math/rotation/Rot.h"
#include "utl/streams/BinStream.h"

namespace {

// Write and read a bone transform as its thirteen floats. The binary folds
// these with the copies in anim/CharSkeleton.o (0x2F9D50, 0x2F98C0); the
// map has them as inline copies in this object.
BinStream& operator<<(BinStream& stream, const CharBoneTransform& xfm) {
    const float* const values = &xfm.mRotation.x;
    for (int i = 0; i < 13; ++i) {
        float value = values[i];
        stream.WriteEndian(&value, 4);
    }
    return stream;
}

BinStream& operator>>(BinStream& stream, CharBoneTransform& xfm) {
    float* const values = &xfm.mRotation.x;
    for (int i = 0; i < 13; ++i) {
        stream.ReadEndian(&values[i], 4);
    }
    return stream;
}

// Whether the float is a NaN or an infinity. Name not in the reference map.
bool IsNaNOrInf(float value) {
    unsigned int bits;
    __builtin_memcpy(&bits, &value, sizeof(bits));
    return (bits & 0x7F800000) == 0x7F800000;
}

// Whether the transform has a value that is not finite, checking the scales
// and the translation before the rotation (the map's CheckNaNOrInf,
// inlined into SetBoneSample).
bool CheckNaNOrInf(const CharBoneTransform& xfm) {
    const float* const values = &xfm.mRotation.x;
    for (int i = 4; i < 13; ++i) {
        if (IsNaNOrInf(values[i])) {
            return true;
        }
    }
    for (int i = 0; i < 4; ++i) {
        if (IsNaNOrInf(values[i])) {
            return true;
        }
    }
    return false;
}

// The unit vector of `v`, or zero for a zero vector. Inlined into
// GetLocalTransformRelativeTo. Name not in the reference map.
void NormalizeVector(Vector3& v) {
    const float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    const float scale = length != 0.0f ? 1.0f / length : 0.0f;
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

}  // namespace

// Reconstructed from eboot.elf at 0x333100. The constructor is inlined.
CharAnimRaw* CharAnimRaw::New() {
    return new CharAnimRaw();
}

// Reconstructed from eboot.elf at 0x333250.
CharAnimRaw::~CharAnimRaw() {}

// Reconstructed from eboot.elf at 0x333330. The bones are read as names,
// flags and parents, then the samples bone by bone.
bool CharAnimRaw::Load(BinStream& stream, bool cached) {
    static_cast<void>(cached);
    int rev = 0;
    stream.ReadEndian(&rev, 4);
    if (rev != 10) {
        return false;
    }
    stream.ReadEndian(&mFramesPerSecond, 4);
    ReadKeyTrack(stream, mBeatTrack);

    unsigned int count = 0;
    stream.ReadEndian(&count, 4);
    mBones.resize(count);
    for (BoneInfo& bone : mBones) {
        stream >> bone.mName;
        bool inverseParentScale = false;
        stream.Read(&inverseParentScale, 1);
        bone.mInverseParentScale = inverseParentScale;
        stream.ReadEndian(&bone.mParent, 4);
    }

    stream.ReadEndian(&count, 4);
    mSamples.resize(count);
    for (eastl::vector<CharBoneTransform>& samples : mSamples) {
        stream.ReadEndian(&count, 4);
        samples.resize(count);
        for (CharBoneTransform& xfm : samples) {
            stream >> xfm;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x333670.
void CharAnimRaw::Save(BinStream& stream, bool cached) const {
    static_cast<void>(cached);
    int value = 10;
    stream.WriteEndian(&value, 4);
    stream.WriteEndian(&mFramesPerSecond, 4);
    stream << mBeatTrack;
    value = static_cast<int>(mBones.size());
    stream.WriteEndian(&value, 4);
    for (const BoneInfo& bone : mBones) {
        stream << bone.mName;
        const bool inverseParentScale = bone.mInverseParentScale;
        stream.Write(&inverseParentScale, 1);
        stream.WriteEndian(&bone.mParent, 4);
    }
    value = static_cast<int>(mSamples.size());
    stream.WriteEndian(&value, 4);
    for (const eastl::vector<CharBoneTransform>& samples : mSamples) {
        value = static_cast<int>(samples.size());
        stream.WriteEndian(&value, 4);
        for (const CharBoneTransform& xfm : samples) {
            stream << xfm;
        }
    }
}

// Reconstructed from eboot.elf at 0x333800.
void CharAnimRaw::Init(unsigned long numBones, unsigned long numFrames, float framesPerSecond, const TKeyTrack<float>& beats) {
    mFramesPerSecond = framesPerSecond;
    mBeatTrack.mInterpolation = beats.mInterpolation;
    const unsigned int count = beats.mKeys.size();
    mBeatTrack.mKeys.Resize(count);
    for (unsigned long i = 0; i < count; ++i) {
        mBeatTrack.mKeys[i] = beats.mKeys[i];
    }
    mBones.resize(numBones);
    mSamples.resize(numBones);
    for (unsigned long bone = 0; bone < numBones; ++bone) {
        mSamples[bone].resize(numFrames);
    }
}

// Reconstructed from eboot.elf at 0x3339A0.
unsigned long CharAnimRaw::FindBoneIndex(Symbol name) const {
    for (unsigned long i = 0; i < mBones.size(); ++i) {
        if (mBones[i].mName == name) {
            return i;
        }
    }
    return static_cast<unsigned long>(-1);
}

// Reconstructed from eboot.elf at 0x3339E0.
void CharAnimRaw::SetBone(unsigned long bone, const char* name, unsigned long parent, bool inverseParentScale, bool inBindPose) {
    BoneInfo& info = mBones[bone];
    info.mName = Symbol(name);
    info.mInverseParentScale = inverseParentScale;
    info.mParent = static_cast<unsigned int>(parent);
    info.mInBindPose = inBindPose;
}

// Reconstructed from eboot.elf at 0x333A60.
bool CharAnimRaw::SetBoneSample(unsigned long bone, unsigned long frame, const CharBoneTransform& xfm) {
    mSamples[bone][frame] = xfm;
    return CheckNaNOrInf(xfm);
}

// Reconstructed from eboot.elf at 0x333C50.
const CharBoneTransform& CharAnimRaw::GetLocalTransform(unsigned long bone, unsigned long frame) const {
    return mSamples[bone][frame];
}

// Reconstructed from eboot.elf at 0x333C70.
void CharAnimRaw::GetWorldXfm(unsigned long bone, unsigned long frame, Transform& xfm) const {
    mSamples[bone][frame].ToTransform(xfm);
    unsigned long parent = mBones[bone].mParent;
    while (parent != bone) {
        Transform parentXfm;
        mSamples[parent][frame].ToTransform(parentXfm);
        Multiply(xfm, parentXfm, xfm);
        bone = parent;
        parent = mBones[parent].mParent;
    }
}

// Reconstructed from eboot.elf at 0x333F10. The relative transform keeps
// the scale MakeScale finds, and its rotation is rebuilt from the y axis
// and the z axis before it becomes a quaternion.
void CharAnimRaw::GetLocalTransformRelativeTo(
    unsigned long bone,
    unsigned long relativeTo,
    unsigned long frame,
    CharBoneTransform& xfm) const {
    if (bone == relativeTo || mBones[bone].mParent == relativeTo) {
        xfm = mSamples[bone][frame];
        return;
    }
    Transform world;
    GetWorldXfm(bone, frame, world);
    Transform parent;
    GetWorldXfm(relativeTo, frame, parent);
    Transform inverse;
    Invert(parent, inverse);
    Transform local;
    Multiply(world, inverse, local);
    MakeScale(local.m, xfm.mScale);
    Hmx::Matrix3& m = local.m;
    NormalizeVector(m.y);
    m.x = {m.y.y * m.z.z - m.y.z * m.z.y, m.y.z * m.z.x - m.y.x * m.z.z, m.y.x * m.z.y - m.y.y * m.z.x};
    NormalizeVector(m.x);
    m.z = {m.x.y * m.y.z - m.x.z * m.y.y, m.x.z * m.y.x - m.x.x * m.y.z, m.x.x * m.y.y - m.x.y * m.y.x};
    xfm.mRotation.Set(m);
    xfm.mTranslation = local.v;
    xfm.mParentScale = {1.0f, 1.0f, 1.0f};
}

// Reconstructed from eboot.elf at 0x3344B0.
void CharAnimRaw::CloneBone(Symbol name, unsigned long source) {
    unsigned long index = FindBoneIndex(name);
    if (index == static_cast<unsigned long>(-1)) {
        index = mBones.size();
        mBones.push_back(mBones[source]);
        mSamples.push_back(eastl::vector<CharBoneTransform>());
    } else {
        mBones[index] = mBones[source];
    }
    if (index != source) {
        mSamples[index] = mSamples[source];
    }
    mBones[index].mName = name;
}
