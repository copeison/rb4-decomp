// anim/CharIkBoneInterpCom.o (0x2F1580 to 0x2F511F). The struct types'
// registrations (the map's PropArray<T>::Init(Symbol), 0x2F3560, 0x2F3710,
// 0x2F38C0 and 0x2F3A70), the property handlers after the identity slots
// (0x2F47F0-0x2F505F) and the empty function at 0x2F4530, which has no
// caller, are the compiler's or not identified. CharIkCom::_InitAsSuperclass
// is emitted here at 0x2F3C20. The object's static initializer is at
// 0x2F5060. The class's registration and factory are emitted in
// anim/AnimInit.o.
#include "anim/char/CharIkBoneInterpCom.h"

#include <cmath>
#include <new>

#include "anim/skeleton/CharSkeleton.h"
#include "anim/skeleton/CharSkeletonPose.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropMetadata.h"
#include "math/matrix/Matrix3.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "render/skeletons/RndSkeletonPoseCom.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"

namespace {

// The -1 of a shared header that the static initializer sets (0x19F90D8).
// Nothing in this object reads it. Name not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The index of the array element the tag's path ends in. Name not in the
// reference map.
unsigned long ElementIndex(const TagArgs& args) {
    return args.mPath[-1].mValue;
}

// The component a tag belongs to. Name not in the reference map.
const CharIkBoneInterp& BoneInterp(const TagArgs& args) {
    return static_cast<const CharIkBoneInterp&>(args.mComponent);
}

// The bone names of the skeleton the entity's RndSkeletonPoseCom poses, as
// allowed values without "none". The pose component must exist. Inlined
// into the four bone properties' allowed values. Name not in the reference
// map.
eastl::vector<AllowedValue<Symbol>> SkeletonBones(const TagArgs& args) {
    const RndSkeletonPoseCom* const pose = args.mComponent.mObject->mEntity->GetRoot()->GetCom<RndSkeletonPoseCom>();
    return pose->GetSkeleton()->GetAllowedValueBones(false);
}

// The labels of the axes, and of the axes and the look-at, for the "axis"
// and "operation" properties. Names not in the reference map.
eastl::vector<AllowedValue<int>> AxisAllowedValues() {
    return eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{CharIkBoneInterp::BoneTarget::kAxisX, String("x axis"), String("the x axis")},
        AllowedValue<int>{CharIkBoneInterp::BoneTarget::kAxisY, String("y axis"), String("the y axis")},
        AllowedValue<int>{CharIkBoneInterp::BoneTarget::kAxisZ, String("z axis"), String("the z axis")}};
}
eastl::vector<AllowedValue<int>> OperationAllowedValues() {
    return eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{CharIkBoneInterp::BoneTarget::kAxisX, String("x axis"), String("blend the x axis")},
        AllowedValue<int>{CharIkBoneInterp::BoneTarget::kAxisY, String("y axis"), String("blend the y axis")},
        AllowedValue<int>{CharIkBoneInterp::BoneTarget::kAxisZ, String("z axis"), String("blend the z axis")},
        AllowedValue<int>{CharIkBoneInterp::BoneTarget::kLookAt, String("look at"), String("look at the source bone")}};
}

// The rotation's row of the axis. Name not in the reference map.
Vector3& AxisRow(Hmx::Matrix3& rotation, int axis) {
    return (&rotation.x)[axis];
}
const Vector3& AxisRow(const Hmx::Matrix3& rotation, int axis) {
    return (&rotation.x)[axis];
}

// Scales the vector to unit length, or to zero when it is zero. The binary
// takes the reciprocal square root with one Newton step. Name not in the
// reference map.
void Normalize(Vector3& v) {
    const float lengthSquared = v.x * v.x + v.y * v.y + v.z * v.z;
    const float length = lengthSquared == 0.0F ? 0.0F : std::sqrt(lengthSquared);
    const float scale = length != 0.0F ? 1.0F / length : 0.0F;
    v.x *= scale;
    v.y *= scale;
    v.z *= scale;
}

Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

constexpr unsigned long kNoBone = static_cast<unsigned long>(-1);

}  // namespace

// The object's statics, in the order of its static initializer (0x2F5060).
Symbol CharIkBoneInterp::sId("CharIkBoneInterp");
Symbol CharIkBoneInterp::sClassName("CharIkBoneInterp");
PropRegistry CharIkBoneInterp::sPropRegistry;
ComMetaData CharIkBoneInterp::sMetaData;

// Reconstructed from eboot.elf at 0x2F1580. Each bone property lists the
// skeleton's bones and has an empty change callback.
void CharIkBoneInterp::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<BoneTarget>(Symbol("BoneTarget"));
    RegisterPropStruct<BoneTarget::PositionSource>(Symbol("BoneTarget::PositionSource"));
    RegisterPropStruct<BoneTarget::AxisSource>(Symbol("BoneTarget::AxisSource"));
    RegisterPropStruct<BoneTarget::AxisSpec>(Symbol("BoneTarget::AxisSpec"));
    _InitAsSuperclass(registry, metadata);
    CharIkBoneInterp prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    PropInfo& targets =
        registry.RegisterPropArray("targets", offsetof(CharIkBoneInterp, mTargets), sizeof(PropArray<BoneTarget>));
    ArrayMetadata& targetsMetadata = static_cast<ArrayMetadata&>(*targets.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    targetsMetadata.mHelp = String("Array of bone targets to ik.");
    PropRegistry::RegisterPropArrayItem(targets, kPropertyStruct, sizeof(BoneTarget));
    GroupMetadata& target = static_cast<GroupMetadata&>(*targetsMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const BoneTarget targetPrototype;
    target.mDisplayName._Set([](const TagArgs& args) {
        return String(BoneInterp(args).mTargets[ElementIndex(args)].mTargetBone);
    });
    PropRegistry& targetMembers = target.mRegistry;

    SymbolMetadata& targetBone = TypeSpecificMetadata(
        targetMembers.RegisterProp("target_bone", offsetof(BoneTarget, mTargetBone), kPropertySymbol, sizeof(Symbol)),
        targetPrototype.mTargetBone);
    targetBone.mDefault = targetPrototype.mTargetBone;
    static_cast<void>(Symbol("prop"));
    targetBone.mHelp = String("Name of the bone to animate through blending other bones");
    targetBone.mAllowedValues._Set([](const TagArgs& args) { return SkeletonBones(args); });
    targetBone.mOnChanged = [](const PropAccessorArgs&) {};

    IntMetadata<unsigned long>& targetIndex = TypeSpecificMetadata(
        targetMembers.RegisterProp(
            "bone_target_index", offsetof(BoneTarget, mBoneTargetIndex), kPropertyUInt64, sizeof(unsigned long)),
        targetPrototype.mBoneTargetIndex);
    targetIndex.mDefault = targetPrototype.mBoneTargetIndex;
    static_cast<void>(Symbol("prop"));
    targetIndex.mIsHidden = true;

    PropInfo& positions = targetMembers.RegisterPropArray(
        "position_calculation",
        offsetof(BoneTarget, mPositionCalculation),
        sizeof(PropArray<BoneTarget::PositionSource>));
    ArrayMetadata& positionsMetadata = static_cast<ArrayMetadata&>(*positions.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    positionsMetadata.mHelp = String("Array of bone sources to blend for position.");
    PropRegistry::RegisterPropArrayItem(positions, kPropertyStruct, sizeof(BoneTarget::PositionSource));
    GroupMetadata& position = static_cast<GroupMetadata&>(*positionsMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const BoneTarget::PositionSource positionPrototype;
    position.mHelp = String("bone name followed by blend weight");
    position.mKind = 1;
    PropRegistry& positionMembers = position.mRegistry;

    SymbolMetadata& positionBone = TypeSpecificMetadata(
        positionMembers.RegisterProp(
            "source_bone", offsetof(BoneTarget::PositionSource, mSourceBone), kPropertySymbol, sizeof(Symbol)),
        positionPrototype.mSourceBone);
    positionBone.mDefault = positionPrototype.mSourceBone;
    static_cast<void>(Symbol("prop"));
    positionBone.mHelp = String("name of the bone to get a position from.");
    positionBone.mAllowedValues._Set([](const TagArgs& args) { return SkeletonBones(args); });
    positionBone.mOnChanged = [](const PropAccessorArgs&) {};

    FloatMetadata& weight = TypeSpecificMetadata(
        positionMembers.RegisterProp(
            "weight", offsetof(BoneTarget::PositionSource, mWeight), kPropertyFloat, sizeof(float)),
        positionPrototype.mWeight);
    weight.mDefault = positionPrototype.mWeight;
    static_cast<void>(Symbol("prop"));
    weight.mHelp = String("weight to apply");
    weight.mMin = 0.0F;
    weight.mMax = 1.0F;
    weight.mDefault = 1.0F;

    IntMetadata<unsigned long>& positionIndex = TypeSpecificMetadata(
        positionMembers.RegisterProp(
            "source_bone_index",
            offsetof(BoneTarget::PositionSource, mSourceBoneIndex),
            kPropertyUInt64,
            sizeof(unsigned long)),
        positionPrototype.mSourceBoneIndex);
    positionIndex.mDefault = positionPrototype.mSourceBoneIndex;
    static_cast<void>(Symbol("prop"));
    positionIndex.mIsHidden = true;

    // The primary axis is x by default and blends look-ats; the secondary
    // is y and blends x axes.
    GroupMetadata& primaryAxis = static_cast<GroupMetadata&>(
        *targetMembers
             .RegisterProp(
                 "primary_axis", offsetof(BoneTarget, mPrimaryAxis), kPropertyStruct, sizeof(BoneTarget::AxisSpec))
             .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    {
        const BoneTarget::AxisSpec specPrototype;
        primaryAxis.mHelp = String("Which axis to make primary and how to calculate it");
        PropRegistry& specMembers = primaryAxis.mRegistry;

        IntMetadata<int>& axis = static_cast<IntMetadata<int>&>(
            *specMembers.RegisterProp("axis", offsetof(BoneTarget::AxisSpec, mAxis), kPropertyInt, sizeof(int))
                 .mMetadata.Get());
        axis.mDefault = static_cast<int>(specPrototype.mAxis);
        static_cast<void>(Symbol("prop"));
        axis.mHelp = String("The primary axis to pose, is posed first, and the other axes will be orthogonal to it");
        axis.mDefault = static_cast<int>(BoneTarget::kAxisX);
        axis.mAllowedValues = AxisAllowedValues();

        PropInfo& sources = specMembers.RegisterPropArray(
            "calculation", offsetof(BoneTarget::AxisSpec, mSources), sizeof(PropArray<BoneTarget::AxisSource>));
        ArrayMetadata& sourcesMetadata = static_cast<ArrayMetadata&>(*sources.mMetadata.Get());
        static_cast<void>(Symbol("array"));
        sourcesMetadata.mHelp = String("Array of bone sources to blend for primary axis.");
        PropRegistry::RegisterPropArrayItem(sources, kPropertyStruct, sizeof(BoneTarget::AxisSource));
        GroupMetadata& source = static_cast<GroupMetadata&>(*sourcesMetadata.mItemInfo.mMetadata.Get());
        static_cast<void>(Symbol("struct_item"));
        const BoneTarget::AxisSource sourcePrototype;
        source.mKind = 1;
        source.mHelp = String("Source bone name, operation to perform, and the blend weight");
        PropRegistry& sourceMembers = source.mRegistry;

        SymbolMetadata& sourceBone = TypeSpecificMetadata(
            sourceMembers.RegisterProp(
                "source_bone", offsetof(BoneTarget::AxisSource, mSourceBone), kPropertySymbol, sizeof(Symbol)),
            sourcePrototype.mSourceBone);
        sourceBone.mDefault = sourcePrototype.mSourceBone;
        static_cast<void>(Symbol("prop"));
        sourceBone.mAllowedValues._Set([](const TagArgs& args) { return SkeletonBones(args); });
        sourceBone.mOnChanged = [](const PropAccessorArgs&) {};

        IntMetadata<int>& operation = static_cast<IntMetadata<int>&>(
            *sourceMembers
                 .RegisterProp("operation", offsetof(BoneTarget::AxisSource, mOperation), kPropertyInt, sizeof(int))
                 .mMetadata.Get());
        operation.mDefault = static_cast<int>(sourcePrototype.mOperation);
        static_cast<void>(Symbol("prop"));
        operation.mDefault = static_cast<int>(BoneTarget::kLookAt);
        operation.mAllowedValues = OperationAllowedValues();

        FloatMetadata& blendWeight = TypeSpecificMetadata(
            sourceMembers.RegisterProp(
                "blend_weight", offsetof(BoneTarget::AxisSource, mBlendWeight), kPropertyFloat, sizeof(float)),
            sourcePrototype.mBlendWeight);
        blendWeight.mDefault = sourcePrototype.mBlendWeight;
        static_cast<void>(Symbol("prop"));
        blendWeight.mHelp = String("the blend weight, can be negative for axes or lookat to negate the direction");
        blendWeight.mDefault = 1.0F;
        blendWeight.mMin = -1.0F;
        blendWeight.mMax = 1.0F;

        IntMetadata<unsigned long>& sourceIndex = TypeSpecificMetadata(
            sourceMembers.RegisterProp(
                "source_bone_index",
                offsetof(BoneTarget::AxisSource, mSourceBoneIndex),
                kPropertyUInt64,
                sizeof(unsigned long)),
            sourcePrototype.mSourceBoneIndex);
        sourceIndex.mDefault = sourcePrototype.mSourceBoneIndex;
        static_cast<void>(Symbol("prop"));
        sourceIndex.mIsHidden = true;
    }

    GroupMetadata& secondaryAxis = static_cast<GroupMetadata&>(
        *targetMembers
             .RegisterProp(
                 "secondary_axis", offsetof(BoneTarget, mSecondaryAxis), kPropertyStruct, sizeof(BoneTarget::AxisSpec))
             .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    {
        const BoneTarget::AxisSpec specPrototype;
        secondaryAxis.mHelp = String("Which axis to make secondary and how to calculate it");
        PropRegistry& specMembers = secondaryAxis.mRegistry;

        IntMetadata<int>& axis = static_cast<IntMetadata<int>&>(
            *specMembers.RegisterProp("axis", offsetof(BoneTarget::AxisSpec, mAxis), kPropertyInt, sizeof(int))
                 .mMetadata.Get());
        axis.mDefault = static_cast<int>(specPrototype.mAxis);
        static_cast<void>(Symbol("prop"));
        axis.mHelp = String("The secondary axis to pose, will be orthogonal to primary axis");
        axis.mDefault = static_cast<int>(BoneTarget::kAxisY);
        axis.mAllowedValues = AxisAllowedValues();

        PropInfo& sources = specMembers.RegisterPropArray(
            "sources", offsetof(BoneTarget::AxisSpec, mSources), sizeof(PropArray<BoneTarget::AxisSource>));
        ArrayMetadata& sourcesMetadata = static_cast<ArrayMetadata&>(*sources.mMetadata.Get());
        static_cast<void>(Symbol("array"));
        sourcesMetadata.mHelp = String("Array of bone sources to blend for secondary axis.");
        PropRegistry::RegisterPropArrayItem(sources, kPropertyStruct, sizeof(BoneTarget::AxisSource));
        GroupMetadata& source = static_cast<GroupMetadata&>(*sourcesMetadata.mItemInfo.mMetadata.Get());
        static_cast<void>(Symbol("struct_item"));
        const BoneTarget::AxisSource sourcePrototype;
        source.mKind = 1;
        source.mHelp = String("Source bone name, operation to perform, and the blend weight");
        PropRegistry& sourceMembers = source.mRegistry;

        SymbolMetadata& sourceBone = TypeSpecificMetadata(
            sourceMembers.RegisterProp(
                "source_bone", offsetof(BoneTarget::AxisSource, mSourceBone), kPropertySymbol, sizeof(Symbol)),
            sourcePrototype.mSourceBone);
        sourceBone.mDefault = sourcePrototype.mSourceBone;
        static_cast<void>(Symbol("prop"));
        sourceBone.mAllowedValues._Set([](const TagArgs& args) { return SkeletonBones(args); });
        sourceBone.mOnChanged = [](const PropAccessorArgs&) {};

        IntMetadata<int>& operation = static_cast<IntMetadata<int>&>(
            *sourceMembers
                 .RegisterProp("operation", offsetof(BoneTarget::AxisSource, mOperation), kPropertyInt, sizeof(int))
                 .mMetadata.Get());
        operation.mDefault = static_cast<int>(sourcePrototype.mOperation);
        static_cast<void>(Symbol("prop"));
        operation.mDefault = static_cast<int>(BoneTarget::kAxisX);
        operation.mAllowedValues = OperationAllowedValues();

        FloatMetadata& blendWeight = TypeSpecificMetadata(
            sourceMembers.RegisterProp(
                "blend_weight", offsetof(BoneTarget::AxisSource, mBlendWeight), kPropertyFloat, sizeof(float)),
            sourcePrototype.mBlendWeight);
        blendWeight.mDefault = sourcePrototype.mBlendWeight;
        static_cast<void>(Symbol("prop"));
        blendWeight.mHelp = String("the blend weight, can be negative for axes or lookat to negate the direction");
        blendWeight.mDefault = 1.0F;
        blendWeight.mMin = -1.0F;
        blendWeight.mMax = 1.0F;

        IntMetadata<unsigned long>& sourceIndex = TypeSpecificMetadata(
            sourceMembers.RegisterProp(
                "source_bone_index",
                offsetof(BoneTarget::AxisSource, mSourceBoneIndex),
                kPropertyUInt64,
                sizeof(unsigned long)),
            sourcePrototype.mSourceBoneIndex);
        sourceIndex.mDefault = sourcePrototype.mSourceBoneIndex;
        static_cast<void>(Symbol("prop"));
        sourceIndex.mIsHidden = true;
    }

    metadata.mDescription =
        "Interpolates bones from other bones.  The way it works is it first computes a position for the bone "
        "through a weighted average of other bones.  Then it computes the primary orientation axis through a "
        "weighted average of other bone axes and lookats.  Then a secondary axis is calculated the same way.  The "
        "secondary axis is made to be perpendicular to the primary axis.  Then the third axis is computed from the "
        "other two.  You can make the weights negative to flip their direction.  For example, a lookat turns into "
        "a lookawayfrom by making the weight negative.  The weights also do not have to sum to 1, they are just "
        "proportional to each other";
    metadata.mAuthor = "James Fleming";
    metadata.mRootOnly = false;
    metadata.mReserved = true;
}

// Reconstructed from eboot.elf at 0x2F3CD0. The deleting destructor is at
// 0x2F4540.
CharIkBoneInterp::~CharIkBoneInterp() {}

// Reconstructed from eboot.elf at 0x2F3D60.
void CharIkBoneInterp::Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton) {
    for (unsigned long index = 0; index < mTargets.size(); ++index) {
        mTargets[index].Apply(pose, skeleton);
    }
}

// Reconstructed from eboot.elf at 0x2F3DD0. The binary picks the third axis
// before checking the bone.
void CharIkBoneInterp::BoneTarget::Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton) const {
    const int primary = mPrimaryAxis.mAxis;
    const int secondary = mSecondaryAxis.mAxis;
    int third;
    if (primary != kAxisX && secondary != kAxisX) {
        third = kAxisX;
    } else if (primary != kAxisY && secondary != kAxisY) {
        third = kAxisY;
    } else if (primary != kAxisZ && secondary != kAxisZ) {
        third = kAxisZ;
    } else {
        return;
    }
    if (mBoneTargetIndex == kNoBone) {
        return;
    }
    Transform xfm = pose.GetObjectXfm(mBoneTargetIndex, skeleton);
    if (mPositionCalculation.size() != 0) {
        xfm.v.x = 0.0F;
        xfm.v.y = 0.0F;
        xfm.v.z = 0.0F;
        float total = 0.0F;
        for (unsigned long index = 0; index < mPositionCalculation.size(); ++index) {
            const PositionSource& source = mPositionCalculation[index];
            if (source.mSourceBoneIndex == kNoBone) {
                continue;
            }
            const Transform& from = pose.GetObjectXfm(source.mSourceBoneIndex, skeleton);
            xfm.v.x += source.mWeight * from.v.x;
            xfm.v.y += source.mWeight * from.v.y;
            xfm.v.z += source.mWeight * from.v.z;
            total += std::fabs(source.mWeight);
        }
        const float scale = 1.0F / total;
        xfm.v.x *= scale;
        xfm.v.y *= scale;
        xfm.v.z *= scale;
    }
    mPrimaryAxis.Apply(xfm, pose, skeleton);
    mSecondaryAxis.Apply(xfm, pose, skeleton);

    Vector3& primaryRow = AxisRow(xfm.m, primary);
    Normalize(primaryRow);
    Vector3& secondaryRow = AxisRow(xfm.m, secondary);
    const float dot =
        secondaryRow.x * primaryRow.x + secondaryRow.y * primaryRow.y + secondaryRow.z * primaryRow.z;
    secondaryRow.x -= primaryRow.x * dot;
    secondaryRow.y -= primaryRow.y * dot;
    secondaryRow.z -= primaryRow.z * dot;
    Normalize(secondaryRow);
    // The third axis completes a right-handed frame.
    const int next = primary == kAxisZ ? kAxisX : primary + 1;
    AxisRow(xfm.m, third) = next == secondary ? Cross(primaryRow, secondaryRow) : Cross(secondaryRow, primaryRow);
    pose.SetObjectXfm(mBoneTargetIndex, xfm, skeleton);
}

// Reconstructed from eboot.elf at 0x2F41B0.
void CharIkBoneInterp::BoneTarget::AxisSpec::Apply(
    Transform& xfm,
    CharSkeletonPose& pose,
    const CharSkeleton* skeleton) const {
    Vector3& axis = AxisRow(xfm.m, mAxis);
    axis.x = 0.0F;
    axis.y = 0.0F;
    axis.z = 0.0F;
    const unsigned long count = mSources.size();
    for (unsigned long index = 0; index < count; ++index) {
        const AxisSource& source = mSources[index];
        if (source.mSourceBoneIndex == kNoBone) {
            continue;
        }
        const Transform& from = pose.GetObjectXfm(source.mSourceBoneIndex, skeleton);
        if (source.mOperation == kLookAt) {
            Vector3 direction = {from.v.x - xfm.v.x, from.v.y - xfm.v.y, from.v.z - xfm.v.z};
            Normalize(direction);
            axis.x += direction.x * source.mBlendWeight;
            axis.y += direction.y * source.mBlendWeight;
            axis.z += direction.z * source.mBlendWeight;
        } else {
            const Vector3& row = AxisRow(from.m, source.mOperation);
            axis.x += source.mBlendWeight * row.x;
            axis.y += source.mBlendWeight * row.y;
            axis.z += source.mBlendWeight * row.z;
        }
    }
}

// Reconstructed from eboot.elf at 0x2F4350.
bool CharIkBoneInterp::_OnResourcesLoaded() {
    const RndSkeletonPoseCom* const pose = mObject->mEntity->GetRoot()->GetCom<RndSkeletonPoseCom>();
    if (pose != nullptr) {
        const CharSkeleton* const skeleton = pose->GetSkeleton();
        if (skeleton != nullptr) {
            _ResolveBones(skeleton);
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2F43D0.
void CharIkBoneInterp::_ResolveBones(const CharSkeleton* skeleton) {
    for (unsigned long index = 0; index < mTargets.size(); ++index) {
        BoneTarget& target = mTargets[index];
        target.mBoneTargetIndex = static_cast<unsigned long>(skeleton->FindBoneIndex(target.mTargetBone));
        for (unsigned long source = 0; source < target.mPositionCalculation.size(); ++source) {
            BoneTarget::PositionSource& position = target.mPositionCalculation[source];
            position.mSourceBoneIndex = static_cast<unsigned long>(skeleton->FindBoneIndex(position.mSourceBone));
        }
        for (unsigned long source = 0; source < target.mPrimaryAxis.mSources.size(); ++source) {
            BoneTarget::AxisSource& axis = target.mPrimaryAxis.mSources[source];
            axis.mSourceBoneIndex = static_cast<unsigned long>(skeleton->FindBoneIndex(axis.mSourceBone));
        }
        for (unsigned long source = 0; source < target.mSecondaryAxis.mSources.size(); ++source) {
            BoneTarget::AxisSource& axis = target.mSecondaryAxis.mSources[source];
            axis.mSourceBoneIndex = static_cast<unsigned long>(skeleton->FindBoneIndex(axis.mSourceBone));
        }
    }
}

// Reconstructed from eboot.elf at 0x2F45E0.
Symbol CharIkBoneInterp::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x2F45F0.
Symbol CharIkBoneInterp::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x2F4600.
int CharIkBoneInterp::CurrentRev() const {
    return const_cast<CharIkBoneInterp*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x2F4620.
bool CharIkBoneInterp::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x2F4650.
Component* CharIkBoneInterp::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x2F4660. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* CharIkBoneInterp::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<CharIkBoneInterp*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) CharIkBoneInterp(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x2F47D0.
PropRegistry& CharIkBoneInterp::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x2F47E0.
ComMetaData& CharIkBoneInterp::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x2B6540. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* CharIkBoneInterp::_Create() {
    return new CharIkBoneInterp();
}

// Reconstructed from eboot.elf at 0x2AF460. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void CharIkBoneInterp::Init() {
    InitComponentClass<CharIkBoneInterp>();
}
