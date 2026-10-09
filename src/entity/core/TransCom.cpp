#include "entity/core/TransCom.h"

#include <cmath>

// The class symbols at 0x19E46E8 and 0x19E46F0, which the registration at
// 0x1AFFB0 assigns.
Symbol TransCom::sId;
Symbol TransCom::sClassName;

namespace {

// The length of the row. The binary takes the reciprocal square root with
// one Newton step and gives zero for a zero row.
float RowLength(const Vector3& row) {
    const float lengthSquared = row.x * row.x + row.y * row.y + row.z * row.z;
    return lengthSquared == 0.0F ? 0.0F : std::sqrt(lengthSquared);
}

// One when the value is positive, minus one otherwise.
float PositiveSign(float value) {
    return 0.0F < value ? 1.0F : -1.0F;
}

Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// The smallest scale the rotation is divided by. Name not in the reference
// map.
constexpr float kMinScale = 0.0001F;

}  // namespace

// Reconstructed from eboot.elf at 0x127730. A left-handed rotation keeps a
// negative z scale. The rotation is then rebuilt as an orthonormal basis
// from the y and z rows before it is converted.
void TransCom::LocalXfm::SetRotation(const Hmx::Matrix3& rotation, bool keepScaleSigns) {
    if (keepScaleSigns) {
        mScale.x = RowLength(rotation.x) * PositiveSign(mScale.x);
        mScale.y = RowLength(rotation.y) * PositiveSign(mScale.y);
        mScale.z = RowLength(rotation.z) * PositiveSign(mScale.z);
        const bool positive = mScale.x * mScale.y * mScale.z >= 0.0F;
        if (IsRightHanded(rotation) != positive) {
            mScale.z = -mScale.z;
        }
    } else {
        mScale.x = RowLength(rotation.x);
        mScale.y = RowLength(rotation.y);
        mScale.z = RowLength(rotation.z);
        if (!IsRightHanded(rotation)) {
            mScale.z = -mScale.z;
        }
    }

    Hmx::Matrix3 basis = Hmx::Matrix3::sID;
    float scaleY = mScale.y;
    if (std::fabs(scaleY) <= kMinScale) {
        scaleY = 0.0F > scaleY ? -kMinScale : kMinScale;
    }
    const float inverseY = 1.0F / scaleY;
    basis.y = {rotation.y.x * inverseY, rotation.y.y * inverseY, rotation.y.z * inverseY};
    const float signZ = 0.0F < mScale.z ? 1.0F : -1.0F;
    const Vector3 z = {rotation.z.x * signZ, rotation.z.y * signZ, rotation.z.z * signZ};
    basis.x = Cross(basis.y, z);
    const float length = RowLength(basis.x);
    const float inverseLength = length != 0.0F ? 1.0F / length : 0.0F;
    basis.x = {basis.x.x * inverseLength, basis.x.y * inverseLength, basis.x.z * inverseLength};
    basis.z = Cross(basis.x, basis.y);
    MakeEuler(basis, mEuler);
}

// Reconstructed from eboot.elf at 0x1B3A90.
void TransCom::SetTransParent(GameObjectId parent, bool keepWorld) {
    mTransParent = parent;
    mDirtyFlags |= keepWorld ? 0x14 : 0x4;
}
