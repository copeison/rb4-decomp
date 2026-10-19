// anim/CharSkeletonPose.o (0x2FA3E0 to 0x2FB6CF). The vector growth at
// 0x2FB460 is the container's; the static initializer at 0x2FB6B0 sets
// shared header globals (0x19F9618-0x19F9620).
#include "anim/skeleton/CharSkeletonPose.h"

#include <math.h>

#include "anim/char/CharBoneStateBuffer.h"
#include "anim/skeleton/CharSkeleton.h"
#include "math/rotation/Quat.h"
#include "render/system/RndDevice.h"

namespace {

// The identity transform. Name not in the reference map.
const Transform kIdentityXfm = {{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, {0.0f, 0.0f, 0.0f}};

// The unit vector of `v`, or zero for a zero vector. Name not in the
// reference map.
void NormalizeVector(Vector3& v) {
    const float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    const float scale = length != 0.0f ? 1.0f / length : 0.0f;
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

// Rebuilds the rotation as an orthonormal basis from its y and z rows.
// Inlined into UpdateTransforms. Name not in the reference map.
void Orthonormalize(Hmx::Matrix3& m) {
    NormalizeVector(m.y);
    m.x = {m.y.y * m.z.z - m.y.z * m.z.y, m.y.z * m.z.x - m.y.x * m.z.z, m.y.x * m.z.y - m.y.y * m.z.x};
    NormalizeVector(m.x);
    m.z = {m.x.y * m.y.z - m.x.z * m.y.y, m.x.z * m.y.x - m.x.x * m.y.z, m.x.x * m.y.y - m.x.y * m.y.x};
}

}  // namespace

// Reconstructed from eboot.elf at 0x2FA3E0.
CharSkeletonPose::CharSkeletonPose()
    : mBones(), mChannelBones(), mRootXfm(kIdentityXfm), mRootResetFrame(static_cast<unsigned long>(-1)) {}

// Reconstructed from eboot.elf at 0x2FA460.
CharSkeletonPose::~CharSkeletonPose() {}

// Reconstructed from eboot.elf at 0x2FA4B0.
void CharSkeletonPose::Reset() {
    mChannelBones.clear();
}

// Reconstructed from eboot.elf at 0x2FA4C0.
void CharSkeletonPose::SetRootXfm(const Transform& xfm) {
    mRootXfm = xfm;
}

// Reconstructed from eboot.elf at 0x2FA510.
void CharSkeletonPose::ResetRootXfm() {
    mRootXfm = kIdentityXfm;
    mRootResetFrame = gRndDevice->mFrameCount;
}

// Reconstructed from eboot.elf at 0x2FA550. The channel map is not cleared
// first; Reset does that.
void CharSkeletonPose::Init(const CharSkeleton* skeleton, const CharBoneStateBuffer* buffer) {
    const unsigned int numBones = static_cast<unsigned int>(skeleton->mNames.size());
    mBones.resize(numBones);
    const CharBoneStateSetFormat& format = buffer->mFormat;
    mChannelBones.reserve(static_cast<unsigned int>(format.mBoneNames.size()));
    for (int type = 0; type < CharBoneChannel::kNumTypes; ++type) {
        const unsigned int first = type > 0 ? format.mBoneEnds[type - 1] : 0;
        const unsigned int count = format.mBoneEnds[type] - first;
        if (count == 0) {
            continue;
        }
        const Symbol* name = &format.mBoneNames[first];
        unsigned int bone = 0;
        for (unsigned int i = 0; i < count; ++i, ++name) {
            while (bone < numBones && skeleton->mNames[bone] != *name) {
                ++bone;
            }
            if (bone < numBones) {
                mChannelBones.push_back(bone);
                ++bone;
            }
        }
    }
    ResetToBindPose(skeleton);
}

// Reconstructed from eboot.elf at 0x2FA820.
void CharSkeletonPose::ResetToBindPose(const CharSkeleton* skeleton) {
    mRootXfm = kIdentityXfm;
    for (unsigned long bone = 0; bone < mBones.size(); ++bone) {
        mBones[bone].mLocal = skeleton->mLocalTransforms[bone];
    }
    for (BoneData& bone : mBones) {
        bone.mDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x2FA960. The single-axis rotations are
// not applied.
void CharSkeletonPose::UpdateTransforms(
    const CharSkeleton* skeleton,
    const CharBoneStateBuffer* buffer,
    const Transform& deltaRoot) {
    static_cast<void>(skeleton);
    const CharBoneStateSetFormat& format = buffer->mFormat;
    const float* const weights = buffer->mWeights;
    const unsigned char* const data = buffer->mData;
    unsigned int channel = 0;

    const Vector3* positions = reinterpret_cast<const Vector3*>(data);
    for (; channel < format.mBoneEnds[CharBoneChannel::kPosition]; ++channel, ++positions) {
        if (weights[channel] != 0.0f) {
            mBones[mChannelBones[channel]].mLocal.mTranslation = *positions;
        }
    }
    const Vector3* scales = reinterpret_cast<const Vector3*>(data + format.mDataEnds[CharBoneChannel::kPosition]);
    for (; channel < format.mBoneEnds[CharBoneChannel::kScale]; ++channel, ++scales) {
        if (weights[channel] != 0.0f) {
            mBones[mChannelBones[channel]].mLocal.mScale = *scales;
        }
    }
    const Vector3* parentScales = reinterpret_cast<const Vector3*>(data + format.mDataEnds[CharBoneChannel::kScale]);
    for (; channel < format.mBoneEnds[CharBoneChannel::kInverseParentScale]; ++channel, ++parentScales) {
        if (weights[channel] != 0.0f) {
            mBones[mChannelBones[channel]].mLocal.mParentScale = *parentScales;
        }
    }
    const Hmx::Quat* rotations =
        reinterpret_cast<const Hmx::Quat*>(data + format.mDataEnds[CharBoneChannel::kInverseParentScale]);
    for (; channel < format.mBoneEnds[CharBoneChannel::kRotation]; ++channel, ++rotations) {
        if (weights[channel] != 0.0f) {
            mBones[mChannelBones[channel]].mLocal.mRotation = *rotations;
        }
    }

    Multiply(deltaRoot, mRootXfm, mRootXfm);
    Orthonormalize(mRootXfm.m);
    for (BoneData& bone : mBones) {
        bone.mDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x2FACB0.
void CharSkeletonPose::FlattenHierarchy(
    const eastl::vector<unsigned int>& parents,
    const eastl::vector<CharBoneTransform>& bones,
    eastl::vector<Transform>& xfms) {
    const unsigned long count = bones.size();
    xfms.resize(count);
    for (unsigned long bone = 0; bone < count; ++bone) {
        bones[bone].ToTransform(xfms[bone]);
        const unsigned long parent = parents[bone];
        if (bone != parent) {
            Multiply(xfms[bone], xfms[parent], xfms[bone]);
        }
    }
}

// Reconstructed from eboot.elf at 0x2FAEB0. The identity of an empty pose
// is a local static (0x19F9640).
const Transform& CharSkeletonPose::GetObjectXfm(unsigned long bone, const CharSkeleton* skeleton) const {
    if (mBones.empty()) {
        static const Transform sIdentity = kIdentityXfm;
        return sIdentity;
    }
    BoneData& data = mBones[bone];
    if (data.mDirty) {
        Transform local;
        data.mLocal.ToTransform(local);
        const unsigned long parent = skeleton->mParentIndices[bone];
        if (parent == bone) {
            Multiply(local, mRootXfm, data.mObject);
        } else {
            Multiply(local, GetObjectXfm(parent, skeleton), data.mObject);
        }
        data.mDirty = false;
    }
    return data.mObject;
}

// Reconstructed from eboot.elf at 0x2FB0D0.
const CharBoneTransform& CharSkeletonPose::GetLocalBoneTrans(unsigned long bone) const {
    return mBones[bone].mLocal;
}

// Reconstructed from eboot.elf at 0x2FB0E0.
void CharSkeletonPose::SetLocalCharBoneTrans(unsigned long bone, const CharBoneTransform& xfm, const CharSkeleton* skeleton) {
    BoneData& data = mBones[bone];
    data.mDirty = true;
    data.mLocal = xfm;
    _SetRecomputeChildren(bone, skeleton);
}

// Reconstructed from eboot.elf at 0x2FB150.
void CharSkeletonPose::_SetRecomputeChildren(unsigned long bone, const CharSkeleton* skeleton) {
    unsigned long count = 0;
    const unsigned int* const children = skeleton->GetChildIndices(bone, count);
    for (unsigned long i = 0; i < count; ++i) {
        const unsigned int child = children[i];
        if (!mBones[child].mDirty) {
            mBones[child].mDirty = true;
            _SetRecomputeChildren(child, skeleton);
        }
    }
}

// Reconstructed from eboot.elf at 0x2FB1F0.
void CharSkeletonPose::SetObjectXfm(unsigned long bone, const Transform& xfm, const CharSkeleton* skeleton) {
    BoneData& data = mBones[bone];
    data.mDirty = false;
    data.mObject = xfm;
    _SetRecomputeChildren(bone, skeleton);
}

// Reconstructed from eboot.elf at 0x2FB290.
void CharSkeletonPose::RecomputeAllWorld(const CharSkeleton* skeleton) {
    for (unsigned long bone = 0; bone < mBones.size(); ++bone) {
        BoneData& data = mBones[bone];
        if (!data.mDirty) {
            continue;
        }
        data.mDirty = false;
        data.mLocal.ToTransform(data.mObject);
        const unsigned long parent = skeleton->mParentIndices[bone];
        if (bone == parent) {
            Multiply(data.mObject, mRootXfm, data.mObject);
        } else {
            Multiply(data.mObject, mBones[parent].mObject, data.mObject);
        }
    }
}
