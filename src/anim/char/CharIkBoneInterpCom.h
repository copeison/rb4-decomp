#pragma once

#include <cstddef>

#include "anim/char/CharIkCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/Symbol.h"

class CharSkeleton;
class CharSkeletonPose;
class Transform;

// The IK component that poses bones from other bones
// (anim/CharIkBoneInterpCom.o, 0x2F1580-0x2F511F; class id
// "CharIkBoneInterp"): "Interpolates bones from other bones. The way it
// works is it first computes a position for the bone through a weighted
// average of other bones. Then it computes the primary orientation axis
// through a weighted average of other bone axes and lookats. Then a
// secondary axis is calculated the same way. The secondary axis is made to
// be perpendicular to the primary axis. Then the third axis is computed
// from the other two. You can make the weights negative to flip their
// direction. For example, a lookat turns into a lookawayfrom by making the
// weight negative. The weights also do not have to sum to 1, they are just
// proportional to each other". The map names the class CharIkBoneInterp.
// The vtable at 0x18F6970 has 43 slots. The object is 88 bytes. Field names
// follow the registry.
class CharIkBoneInterp : public CharIkCom {
public:
    // One bone to pose, an element of "targets" (152 bytes).
    struct BoneTarget {
        // A bone whose position counts toward the target's, an element of
        // "position_calculation": "bone name followed by blend weight" (24
        // bytes).
        struct PositionSource {
            // "source_bone": "name of the bone to get a position from."
            Symbol mSourceBone;
            // "weight": "weight to apply", 0 to 1.
            float mWeight = 1.0F;
            // "source_bone_index", hidden: the bone's index in the skeleton,
            // or -1.
            unsigned long mSourceBoneIndex = static_cast<unsigned long>(-1);
        };

        // The axes and the look-at an AxisSource takes. The enumerator
        // names follow the labels and are not in the reference map.
        enum Operation : int {
            // "blend the x axis", and likewise for y and z. As an AxisSpec's
            // "axis", the axis to pose ("the x axis", ...).
            kAxisX = 0,
            kAxisY = 1,
            kAxisZ = 2,
            // "look at the source bone".
            kLookAt = 3,
        };

        // A bone that counts toward an axis, an element of an AxisSpec's
        // array: "Source bone name, operation to perform, and the blend
        // weight" (24 bytes).
        struct AxisSource {
            // "source_bone".
            Symbol mSourceBone;
            // "operation": the source's axis to blend, or the direction to
            // the source.
            Operation mOperation = kLookAt;
            // "blend_weight": "the blend weight, can be negative for axes or
            // lookat to negate the direction", -1 to 1.
            float mBlendWeight = 1.0F;
            // "source_bone_index", hidden: the bone's index, or -1.
            unsigned long mSourceBoneIndex = static_cast<unsigned long>(-1);
        };

        // One axis to pose and its sources, "primary_axis" and
        // "secondary_axis" (48 bytes).
        struct AxisSpec {
            // Sets the transform's row of the axis to the weighted sum of
            // the sources' axes and the directions from the transform's
            // position to the look-at sources. Sources without a bone are
            // skipped.
            void Apply(Transform& xfm, CharSkeletonPose& pose, const CharSkeleton* skeleton) const;  // 0x2F41B0

            // "axis", the row of the bone's rotation to pose.
            Operation mAxis = kAxisX;
            // "calculation" for the primary axis, "sources" for the
            // secondary.
            PropArray<AxisSource> mSources;
        };

        // Poses the bone in the skeleton's space: the weighted average of
        // the position sources (by the weights' magnitudes), the primary
        // axis, the secondary axis made perpendicular to it and the third
        // axis their cross product, each normalized. Nothing happens
        // without a bone, or when the axes leave no third axis.
        void Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton) const;  // 0x2F3DD0

        // "target_bone": "Name of the bone to animate through blending
        // other bones".
        Symbol mTargetBone;
        // "bone_target_index", hidden: the bone's index, or -1.
        unsigned long mBoneTargetIndex = static_cast<unsigned long>(-1);
        // "position_calculation": "Array of bone sources to blend for
        // position."
        PropArray<PositionSource> mPositionCalculation;
        // "primary_axis": "Which axis to make primary and how to calculate
        // it"; x by default.
        AxisSpec mPrimaryAxis;
        // "secondary_axis": "Which axis to make secondary and how to
        // calculate it"; y by default.
        AxisSpec mSecondaryAxis;
    };

    // Inlined into the factory (0x2B6540).
    CharIkBoneInterp() : mResolved(false), mTargets() {}
    // Copies the component for an imprint. Inlined into _Imprint
    // (0x2F4660).
    CharIkBoneInterp(const CharIkBoneInterp& other)
        : CharIkCom(other), mResolved(false), mTargets(other.mTargets) {}
    // Slots 0-1: 0x2F3CD0, 0x2F4540.
    ~CharIkBoneInterp() override;

    Symbol GetId() const override;          // slot 4: 0x2F45E0
    Symbol GetClassName() const override;   // slot 5: 0x2F45F0
    int CurrentRev() const override;        // slot 7: 0x2F4600
    bool IsA(Symbol type) const override;   // slot 8: 0x2F4620
    Component* AsComponent() override;      // slot 9: 0x2F4650
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2F4660
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2F47D0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2F47E0
    // Slot 29: finds the bones' indices in the skeleton of the entity's
    // RndSkeletonPoseCom when it has one. The map's
    // _LoadResources(ObjPtr const&) has this body.
    bool _OnResourcesLoaded() override;  // 0x2F4350
    // Slot 41: poses each target in order.
    void Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton) override;  // 0x2F3D60

    // Sets the bone indices of the targets and their sources from the
    // skeleton's names (-1 for a missing bone). Name not in the reference
    // map.
    void _ResolveBones(const CharSkeleton* skeleton);  // 0x2F43D0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in anim/AnimInit.o at 0x2AF460.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B6540; the
    // placement factory beside it is at 0x2B65C0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2F1580

    // "CharIkBoneInterp", constructed by the static initializer at
    // 0x2F5060, which first sets a shared header's null object id to -1
    // (0x19F90D8).
    static Symbol sId;  // 0x19F90E0
    // Also "CharIkBoneInterp". Name not in the reference map.
    static Symbol sClassName;           // 0x19F90E8
    static PropRegistry sPropRegistry;  // 0x19F90F0
    static ComMetaData sMetaData;       // 0x19F9190

    // Cleared by the constructors; nothing in this object reads or sets
    // it. Name not in the reference map; the evidence is weak.
    bool mResolved;
    // "targets": "Array of bone targets to ik."
    PropArray<BoneTarget> mTargets;
};

static_assert(offsetof(CharIkBoneInterp::BoneTarget::PositionSource, mWeight) == 8);
static_assert(offsetof(CharIkBoneInterp::BoneTarget::PositionSource, mSourceBoneIndex) == 16);
static_assert(sizeof(CharIkBoneInterp::BoneTarget::PositionSource) == 24);
static_assert(offsetof(CharIkBoneInterp::BoneTarget::AxisSource, mOperation) == 8);
static_assert(offsetof(CharIkBoneInterp::BoneTarget::AxisSource, mBlendWeight) == 12);
static_assert(offsetof(CharIkBoneInterp::BoneTarget::AxisSource, mSourceBoneIndex) == 16);
static_assert(sizeof(CharIkBoneInterp::BoneTarget::AxisSource) == 24);
static_assert(offsetof(CharIkBoneInterp::BoneTarget::AxisSpec, mSources) == 8);
static_assert(sizeof(CharIkBoneInterp::BoneTarget::AxisSpec) == 48);
static_assert(offsetof(CharIkBoneInterp::BoneTarget, mBoneTargetIndex) == 8);
static_assert(offsetof(CharIkBoneInterp::BoneTarget, mPositionCalculation) == 16);
static_assert(offsetof(CharIkBoneInterp::BoneTarget, mPrimaryAxis) == 56);
static_assert(offsetof(CharIkBoneInterp::BoneTarget, mSecondaryAxis) == 104);
static_assert(sizeof(CharIkBoneInterp::BoneTarget) == 152);
static_assert(offsetof(CharIkBoneInterp, mResolved) == 40);
static_assert(offsetof(CharIkBoneInterp, mTargets) == 48);
static_assert(sizeof(CharIkBoneInterp) == 88);
