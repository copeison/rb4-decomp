// anim/CharAnimBuffer.o (0x331A50 to 0x3330FF).
#include "anim/char/CharAnimBuffer.h"

#include <math.h>

#include "anim/skeleton/CharSkeleton.h"
#include "math/rotation/Quat.h"

namespace {

// Walks the set's channels of one kind together with the buffer's channels
// of the kind `bufferType` that bear the same bone names. The buffer holds
// the set's channels in the same order, so each name is searched from the
// previous match on, without a bound. `apply` gets the buffer's channel
// data, the set's and the buffer channel's weight. Inlined for every kind.
// Name not in the reference map.
template <class Fn>
void ForEachSharedChannel(
    const CharBoneStateBuffer& buffer,
    int bufferType,
    unsigned int bufferStride,
    const CharBoneStateSet& set,
    int setType,
    unsigned int setStride,
    Fn apply) {
    const CharBoneStateSetFormat& format = *set.mFormat;
    const unsigned int first = setType > 0 ? format.mBoneEnds[setType - 1] : 0;
    const unsigned int count = format.mBoneEnds[setType] - first;
    if (count == 0) {
        return;
    }
    const unsigned int bufferFirst = bufferType > 0 ? buffer.mFormat.mBoneEnds[bufferType - 1] : 0;
    const Symbol* const names = buffer.mFormat.mBoneNames.begin() + bufferFirst;
    const Symbol* match = names;
    const unsigned char* source = set.mData + (setType > 0 ? format.mDataEnds[setType - 1] : 0);
    unsigned char* const data = buffer.mData + (bufferType > 0 ? buffer.mFormat.mDataEnds[bufferType - 1] : 0);
    for (unsigned int i = 0; i < count; ++i, source += setStride) {
        const Symbol name = format.mBoneNames[first + i];
        while (*match != name) {
            ++match;
        }
        const unsigned long index = match - names;
        apply(data + index * bufferStride, source, buffer.mWeights[bufferFirst + index]);
    }
}

// Adds a weighted three-float channel. Name not in the reference map.
void AddVector(unsigned char* data, const unsigned char* source, float& channelWeight, float weight) {
    float* const to = reinterpret_cast<float*>(data);
    const float* const from = reinterpret_cast<const float*>(source);
    to[0] += weight * from[0];
    to[1] += weight * from[1];
    to[2] += weight * from[2];
    channelWeight += weight;
}

// Adds a weighted (axis, w) pair of a single-axis rotation onto a full
// rotation, on its hemisphere. Name not in the reference map.
void AddAxisRotation(float* to, int axis, const float* from, float& channelWeight, float weight) {
    const float sign = to[axis] * from[0] + to[3] * from[1] >= 0.0f ? weight : -weight;
    to[axis] += sign * from[0];
    to[3] += sign * from[1];
    channelWeight += weight;
}

// Scales the components of an accumulated channel so its weight becomes
// `weight`; an empty channel stays. Name not in the reference map.
void ScaleChannel(float* to, const int* components, int count, float& channelWeight, float weight) {
    if (channelWeight == 0.0f) {
        return;
    }
    const float scale = weight / channelWeight;
    for (int i = 0; i < count; ++i) {
        to[components[i]] *= scale;
    }
    channelWeight = weight;
}

}  // namespace

// Reconstructed from eboot.elf at 0x331A50.
CharAnimBuffer::CharAnimBuffer()
    : mAbsolute(), mRelative(), mDeltaRootXfm(), mDeltaRootWeight(0.0f), mSkeleton(nullptr), mSkeletonPath() {}

// Reconstructed from eboot.elf at 0x331AB0.
CharAnimBuffer::~CharAnimBuffer() {}

// Reconstructed from eboot.elf at 0x331AD0.
void CharAnimBuffer::Init(const CharBoneStateSetFormat& format, const CharSkeleton* skeleton, ResourcePath skeletonPath) {
    mAbsolute.Init(format);
    mRelative.Init(format);
    mSkeleton = skeleton;
    mSkeletonPath = skeletonPath;
}

// Reconstructed from eboot.elf at 0x331B10.
void CharAnimBuffer::BeginFrame() {
    mAbsolute.ClearChannelData();
    mRelative.ClearChannelData();
    mDeltaRootXfm.v = {0.0f, 0.0f, 0.0f};
    mDeltaRootXfm.q = {0.0f, 0.0f, 0.0f, 0.0f};
    mDeltaRootWeight = 0.0f;
}

// Reconstructed from eboot.elf at 0x331B40.
void CharAnimBuffer::GetDeltaRootXfm(Transform& xfm) {
    xfm.v = mDeltaRootXfm.v;
    MakeRotMatrix(mDeltaRootXfm.q, xfm.m);
}

// Reconstructed from eboot.elf at 0x332340.
void CharAnimBuffer::GrabBone(unsigned long bone, QuatXfm& xfm) const {
    const Vector3* const positions = reinterpret_cast<const Vector3*>(mAbsolute.mData);
    const Hmx::Quat* const rotations = reinterpret_cast<const Hmx::Quat*>(
        mAbsolute.mData + mAbsolute.mFormat.mDataEnds[CharBoneChannel::kInverseParentScale]);
    xfm.v = positions[bone];
    xfm.q = rotations[bone];
}

// Reconstructed from eboot.elf at 0x332380.
void CharAnimBuffer::GrabAndClearRootBones(QuatXfm* xfms) {
    unsigned long count = 0;
    Vector3* const positions = reinterpret_cast<Vector3*>(mAbsolute.mData);
    const unsigned int* const roots = mSkeleton->GetRootIndices(count);
    Hmx::Quat* const rotations = reinterpret_cast<Hmx::Quat*>(
        mAbsolute.mData + mAbsolute.mFormat.mDataEnds[CharBoneChannel::kInverseParentScale]);
    for (unsigned long i = 0; i < count; ++i) {
        const unsigned int bone = roots[i];
        xfms[i].v = positions[bone];
        xfms[i].q = rotations[bone];
        positions[bone] = {0.0f, 0.0f, 0.0f};
        rotations[bone] = {0.0f, 0.0f, 0.0f, 0.0f};
    }
}

// Reconstructed from eboot.elf at 0x332460.
void CharAnimBuffer::SetRootBones(QuatXfm* xfms) {
    unsigned long count = 0;
    Vector3* const positions = reinterpret_cast<Vector3*>(mAbsolute.mData);
    const unsigned int* const roots = mSkeleton->GetRootIndices(count);
    Hmx::Quat* const rotations = reinterpret_cast<Hmx::Quat*>(
        mAbsolute.mData + mAbsolute.mFormat.mDataEnds[CharBoneChannel::kInverseParentScale]);
    for (unsigned long i = 0; i < count; ++i) {
        const unsigned int bone = roots[i];
        positions[bone] = xfms[i].v;
        rotations[bone] = xfms[i].q;
    }
}

// Reconstructed from eboot.elf at 0x331B70. The absolute channels are
// normalized as they are, the relative ones additively. The root motion is
// divided by its weight from 1 on; below that its rotation takes the
// missing weight from the identity. The relative channels are then added
// onto the absolute ones (the map's _ComposeBuffers, inlined): an absolute
// channel no animation set starts from the skeleton's bind transform, taken
// by the channel's place among its kind, as the driver's buffers hold every
// bone in skeleton order. Positions add, scales multiply and rotations
// compose; single-axis rotations are left out.
void CharAnimBuffer::EndFrame() {
    mAbsolute.NormalizeChannelData(false);
    mRelative.NormalizeChannelData(true);
    const float weight = mDeltaRootWeight;
    Hmx::Quat& q = mDeltaRootXfm.q;
    if (weight == 0.0f) {
        q = {0.0f, 0.0f, 0.0f, 1.0f};
    } else if (weight >= 1.0f) {
        const float scale = 1.0f / weight;
        mDeltaRootXfm.v.x *= scale;
        mDeltaRootXfm.v.y *= scale;
        mDeltaRootXfm.v.z *= scale;
        q.x *= scale;
        q.y *= scale;
        q.z *= scale;
        q.w *= scale;
    } else {
        q.w += q.w < 0.0f ? weight - 1.0f : 1.0f - weight;
    }

    const CharBoneStateSetFormat& absolute = mAbsolute.mFormat;
    const CharBoneStateSetFormat& relative = mRelative.mFormat;
    const Symbol* const names = absolute.mBoneNames.begin();
    const CharBoneTransform* const bind = mSkeleton->mLocalTransforms.begin();
    for (int type = CharBoneChannel::kPosition; type <= CharBoneChannel::kRotation; ++type) {
        const unsigned int first = type > 0 ? relative.mBoneEnds[type - 1] : 0;
        const unsigned int count = relative.mBoneEnds[type] - first;
        const unsigned int absoluteFirst = type > 0 ? absolute.mBoneEnds[type - 1] : 0;
        const unsigned int relativeData = type > 0 ? relative.mDataEnds[type - 1] : 0;
        const unsigned int absoluteData = type > 0 ? absolute.mDataEnds[type - 1] : 0;
        const unsigned int stride = type == CharBoneChannel::kRotation ? 16 : 12;
        const Symbol* match = names + absoluteFirst;
        for (unsigned int k = 0; k < count; ++k) {
            if (mRelative.mWeights[first + k] == 0.0f) {
                continue;
            }
            const Symbol name = relative.mBoneNames[first + k];
            while (*match != name) {
                ++match;
            }
            const unsigned long index = match - names;
            float* const to =
                reinterpret_cast<float*>(mAbsolute.mData + absoluteData + (index - absoluteFirst) * stride);
            const float* const from = reinterpret_cast<const float*>(mRelative.mData + relativeData + k * stride);
            const bool unset = mAbsolute.mWeights[index] == 0.0f;
            if (unset) {
                mAbsolute.mWeights[index] = 1.0f;
            }
            if (type == CharBoneChannel::kPosition) {
                const Vector3 base = unset ? bind[k].mTranslation : Vector3{to[0], to[1], to[2]};
                to[0] = base.x + from[0];
                to[1] = base.y + from[1];
                to[2] = base.z + from[2];
            } else if (type != CharBoneChannel::kRotation) {
                const Vector3& bindScale = type == CharBoneChannel::kScale ? bind[k].mScale : bind[k].mParentScale;
                const Vector3 base = unset ? bindScale : Vector3{to[0], to[1], to[2]};
                to[0] = base.x * from[0];
                to[1] = base.y * from[1];
                to[2] = base.z * from[2];
            } else {
                const Hmx::Quat a = unset ? bind[k].mRotation : Hmx::Quat{to[0], to[1], to[2], to[3]};
                const Hmx::Quat b = {from[0], from[1], from[2], from[3]};
                to[0] = (b.x * a.w - b.y * a.z) + (b.z * a.y + b.w * a.x);
                to[1] = ((b.y * a.w + b.x * a.z) - b.z * a.x) + b.w * a.y;
                to[2] = (b.y * a.x - b.x * a.y) + (b.z * a.w + b.w * a.z);
                to[3] = (b.w * a.w - b.x * a.x) - (b.z * a.z + b.y * a.y);
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x332510. Positions, scales and inverse
// parent scales add weighted; rotations add weighted on the accumulated
// quaternion's hemisphere, and a single-axis rotation adds its (axis, w)
// pair onto the bone's full rotation. Every channel adds the weight.
void CharAnimBuffer::Apply(const CharBoneStateSet& set, float weight, bool relative) {
    CharBoneStateBuffer& buffer = relative ? mRelative : mAbsolute;
    for (int type = CharBoneChannel::kPosition; type <= CharBoneChannel::kInverseParentScale; ++type) {
        ForEachSharedChannel(
            buffer, type, 12, set, type, 12, [weight](unsigned char* to, const unsigned char* from, float& w) {
                AddVector(to, from, w, weight);
            });
    }
    ForEachSharedChannel(
        buffer,
        CharBoneChannel::kRotation,
        16,
        set,
        CharBoneChannel::kRotation,
        16,
        [weight](unsigned char* to, const unsigned char* from, float& w) {
            float* const q = reinterpret_cast<float*>(to);
            const float* const r = reinterpret_cast<const float*>(from);
            const float sign = q[0] * r[0] + q[1] * r[1] + q[2] * r[2] + q[3] * r[3] >= 0.0f ? weight : -weight;
            q[0] += sign * r[0];
            q[1] += sign * r[1];
            q[2] += sign * r[2];
            q[3] += sign * r[3];
            w += weight;
        });
    for (int axis = 0; axis < 3; ++axis) {
        ForEachSharedChannel(
            buffer,
            CharBoneChannel::kRotation,
            16,
            set,
            CharBoneChannel::kRotationX + axis,
            8,
            [weight, axis](unsigned char* to, const unsigned char* from, float& w) {
                AddAxisRotation(reinterpret_cast<float*>(to), axis, reinterpret_cast<const float*>(from), w, weight);
            });
    }
}

// Reconstructed from eboot.elf at 0x332AE0. Rotations scale whole; a
// single-axis rotation scales its axis component and w.
void CharAnimBuffer::ScaleDown(const CharBoneStateSet& set, float weight, bool relative) {
    CharBoneStateBuffer& buffer = relative ? mRelative : mAbsolute;
    static const int kVector[3] = {0, 1, 2};
    static const int kQuat[4] = {0, 1, 2, 3};
    for (int type = CharBoneChannel::kPosition; type <= CharBoneChannel::kInverseParentScale; ++type) {
        ForEachSharedChannel(
            buffer, type, 12, set, type, 12, [weight](unsigned char* to, const unsigned char* from, float& w) {
                static_cast<void>(from);
                ScaleChannel(reinterpret_cast<float*>(to), kVector, 3, w, weight);
            });
    }
    ForEachSharedChannel(
        buffer,
        CharBoneChannel::kRotation,
        16,
        set,
        CharBoneChannel::kRotation,
        16,
        [weight](unsigned char* to, const unsigned char* from, float& w) {
            static_cast<void>(from);
            ScaleChannel(reinterpret_cast<float*>(to), kQuat, 4, w, weight);
        });
    for (int axis = 0; axis < 3; ++axis) {
        const int components[2] = {axis, 3};
        ForEachSharedChannel(
            buffer,
            CharBoneChannel::kRotation,
            16,
            set,
            CharBoneChannel::kRotationX + axis,
            8,
            [weight, &components](unsigned char* to, const unsigned char* from, float& w) {
                static_cast<void>(from);
                ScaleChannel(reinterpret_cast<float*>(to), components, 2, w, weight);
            });
    }
}

// Reconstructed from eboot.elf at 0x332F90.
void CharAnimBuffer::ApplyDeltaRootXfm(const QuatXfm& xfm, float weight) {
    mDeltaRootWeight += weight;
    mDeltaRootXfm.v.x += weight * xfm.v.x;
    mDeltaRootXfm.v.y += weight * xfm.v.y;
    mDeltaRootXfm.v.z += weight * xfm.v.z;
    const float scale = fabsf(weight);
    const Hmx::Quat q = {xfm.q.x * scale, xfm.q.y * scale, xfm.q.z * scale, xfm.q.w * weight};
    Hmx::Quat& sum = mDeltaRootXfm.q;
    if (sum.y * q.y + sum.x * q.x + sum.z * q.z + sum.w * q.w < 0.0f) {
        sum.x -= q.x;
        sum.y -= q.y;
        sum.z -= q.z;
        sum.w -= q.w;
    } else {
        sum.x += q.x;
        sum.y += q.y;
        sum.z += q.z;
        sum.w += q.w;
    }
}

// Reconstructed from eboot.elf at 0x3330E0.
void CharAnimBuffer::ScaleDownDeltaRootXfm(float scale) {
    mDeltaRootXfm.v.x *= scale;
    mDeltaRootXfm.v.y *= scale;
    mDeltaRootXfm.v.z *= scale;
    mDeltaRootXfm.q.x *= scale;
    mDeltaRootXfm.q.y *= scale;
    mDeltaRootXfm.q.z *= scale;
    mDeltaRootXfm.q.w *= scale;
    mDeltaRootWeight *= scale;
}
