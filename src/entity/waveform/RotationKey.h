#pragma once

#include <cstddef>

#include "entity/props/PropMetadata.h"
#include "entity/waveform/Key.h"
#include "math/rotation/Quat.h"
#include "math/rotation/Rot.h"
#include "math/vector/Vector3.h"

// The key values of the rotation and transform property animations
// (PropKeysRotationCom, PropKeysTransformSRTCom), whose samplings live in
// entity/Key.o. Both types are newer than the reference map. A key keeps
// the rotation both as Euler angles, which the property and the editor
// read, and as a quaternion, which the samplers blend; a flag marks the
// form that is stale, and the readers refresh it on use, so the members
// are mutable. This header has no object of its own.

// The value of a rotation key (32 bytes): Euler angles in radians, their
// quaternion and the stale flags. Name and field names not in the
// reference map; the type's name is the registration's "TKey<RotationKey>".
struct RotationKey {
    // Which form is out of date. Names not in the reference map.
    enum Flags : unsigned char {
        kEulerStale = 1,
        kQuatStale = 2,
    };

    // No rotation.
    RotationKey() : mEuler{0.0f, 0.0f, 0.0f}, mQuat{0.0f, 0.0f, 0.0f, 1.0f}, mFlags(0) {}
    // The rotation of the Euler angles, its quaternion computed. Inlined
    // into the rotation keys' users (0x315B70, 0x317020).
    explicit RotationKey(const Vector3& euler) : mEuler(euler), mQuat{0.0f, 0.0f, 0.0f, 1.0f}, mFlags(0) {
        mQuat.Set(mEuler);
        mFlags &= ~kQuatStale;
    }
    // The quaternion's rotation, its Euler angles computed. Inlined into
    // PropAnimResource::_LoadEntity (0x2FD3A0), which converts old
    // quaternion keys.
    explicit RotationKey(const Hmx::Quat& quat) : RotationKey() {
        mQuat = quat;
        mFlags = 0;
        MakeEuler(mQuat, mEuler);
        mFlags &= ~kEulerStale;
    }

    // Recomputes the Euler angles when they are stale. Inlined into its
    // users. Name not in the reference map.
    void UpdateEuler() const {
        if ((mFlags & kEulerStale) != 0) {
            MakeEuler(mQuat, mEuler);
            mFlags &= ~kEulerStale;
        }
    }

    mutable Vector3 mEuler;
    mutable Hmx::Quat mQuat;
    mutable unsigned char mFlags;
};

static_assert(offsetof(RotationKey, mQuat) == 12);
static_assert(offsetof(RotationKey, mFlags) == 28);
static_assert(sizeof(RotationKey) == 32);
static_assert(sizeof(TKey<RotationKey>) == 36);

// The value of a transform key (56 bytes): the scale, Euler angles and
// translation, the rotation's quaternion and the stale flags, as
// RotationKey's. Name and field names not in the reference map; the type's
// name is the registration's "TKey<TransformSRTKey>".
struct TransformSRTKey {
    // The identity transform.
    TransformSRTKey()
        : mXfm{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
          mQuat{0.0f, 0.0f, 0.0f, 1.0f},
          mFlags(0) {}
    // The transform, its rotation's quaternion computed. Inlined into the
    // transform keys' users (0x31F450, 0x321120).
    explicit TransformSRTKey(const PropXfm& xfm) : mXfm(xfm), mQuat{0.0f, 0.0f, 0.0f, 1.0f}, mFlags(0) {
        mQuat.Set(mXfm.mRotation);
        mFlags &= ~RotationKey::kQuatStale;
    }

    // Recomputes the Euler angles when they are stale. Inlined into its
    // users. Name not in the reference map.
    void UpdateEuler() const {
        if ((mFlags & RotationKey::kEulerStale) != 0) {
            MakeEuler(mQuat, mXfm.mRotation);
            mFlags &= ~RotationKey::kEulerStale;
        }
    }

    mutable PropXfm mXfm;
    mutable Hmx::Quat mQuat;
    mutable unsigned char mFlags;
};

static_assert(offsetof(TransformSRTKey, mQuat) == 36);
static_assert(offsetof(TransformSRTKey, mFlags) == 52);
static_assert(sizeof(TransformSRTKey) == 56);
static_assert(sizeof(TKey<TransformSRTKey>) == 60);

// With the interpolation's 0x40000000 flag, the Euler angles blend one by
// one (InterpEulerLinear) and the quaternion goes stale; otherwise the
// quaternions blend (FastInterp) and the angles go stale. Both inputs are
// refreshed first. Not reconstructed.
template <>
void TKeyTrack<RotationKey>::_SampleLinearInterpValue(
    const TKey<RotationKey>* prev,
    const TKey<RotationKey>* next,
    float t,
    RotationKey& value) const;  // 0x1DD340
// The spline through the neighbouring keys' rotations; its tangent helper
// is 0x1E0AD0. Not reconstructed.
template <>
void TKeyTrack<RotationKey>::_SampleSplineInterpValue(
    const TKey<RotationKey>* prev,
    const TKey<RotationKey>* next,
    float t,
    RotationKey& value) const;  // 0x1DD4B0

// With the 0x40000000 flag, InterpXfmLinear; otherwise the scale and
// translation blend linearly and the quaternions through FastInterp. Not
// reconstructed.
template <>
void TKeyTrack<TransformSRTKey>::_SampleLinearInterpValue(
    const TKey<TransformSRTKey>* prev,
    const TKey<TransformSRTKey>* next,
    float t,
    TransformSRTKey& value) const;  // 0x1DD9B0
// The spline through the neighbouring keys' transforms. Not reconstructed.
template <>
void TKeyTrack<TransformSRTKey>::_SampleSplineInterpValue(
    const TKey<TransformSRTKey>* prev,
    const TKey<TransformSRTKey>* next,
    float t,
    TransformSRTKey& value) const;  // 0x1DDC50
