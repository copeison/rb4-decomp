// entity/TransCom.o (0x1AFFB0 to 0x1BAA91) and TransCom::LocalXfm::SetRotation
// (0x127730). The handler (0x1B47F0), the game-mode enter (0x1B4040) and
// the poll (0x1B4140) are not reconstructed.
#include "entity/core/TransCom.h"

#include <cmath>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "math/rotation/Rot.h"
#include "utl/text/MakeString.h"

namespace {

// The id of no object, at 0x19E46E0; the object's static initializer
// (0x1BA9D0) sets it. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};
// The object "utilities/copy_world_xfm/from" names, shared by every
// TransCom (0x19E46E4). Name not in the reference map.
GameObjectId gCopyWorldXfmSource = {0xFFFFFFFFu};

}  // namespace

// The class statics, in the order of the static initializer (0x1BA9D0).
Symbol TransCom::sId("Transform");
Symbol TransCom::sClassName("Transform");
PropRegistry TransCom::sPropRegistry;
ComMetaData TransCom::sMetaData;

namespace {

// The identity rotation and transform the class's code builds from
// constants. Names not in the reference map.
const Hmx::Matrix3 kIdentityRotation = {{1.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F, 1.0F}};
const Transform kIdentityXfm = {kIdentityRotation, {0.0F, 0.0F, 0.0F}};

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

// The smallest scale the rotation is divided by, and the smallest the
// scale properties accept. Name not in the reference map.
constexpr float kMinScale = 0.0001F;

// The scale a scale property stores: magnitudes up to kMinScale become
// kMinScale with the value's sign. Inlined into the setters of "x_scale",
// "y_scale" and "z_scale". Name not in the reference map.
float ClampScale(float scale) {
    if (std::fabs(scale) <= kMinScale) {
        return 0.0F > scale ? -kMinScale : kMinScale;
    }
    return scale;
}

// Whether the component is on its entity's root object, whose transform
// the editor does not change. Inlined into the tag functions. Name not in
// the reference map.
bool IsRootObject(const Component& component) {
    return component.mObject->mEntity->GetRoot() == component.mObject;
}

// The help of a local transform property, marked for the root object.
// Inlined into the help functions. Name not in the reference map.
String LocalTransformHelp(const TagArgs& args, const char* help) {
    const char* const text =
        IsRootObject(args.mComponent) ? MakeString("%s (disabled for root object)", help) : help;
    return String(text);
}

// One coordinate of the vector. Name not in the reference map.
float Coordinate(const Vector3& vector, int index) {
    return index == 0 ? vector.x : index == 1 ? vector.y : vector.z;
}

// An element of the local transform's scaled rotation matrix, for the
// read-only "local_transform" axes. The binary emits one getter per
// element, from 0x1B5130 (x_axis.x) to 0x1B5580 (z_axis.z); each builds the
// whole matrix row by row up to its element. Name not in the reference map.
template <int Row, int Column>
float GetLocalAxisElement(const PropAccessorArgs& args) {
    const TransCom::LocalXfm& xfm = static_cast<const TransCom*>(args.mComponent)->mLocalXfm;
    Hmx::Matrix3 rotation = kIdentityRotation;
    MakeRotMatrix(xfm.mEuler, rotation, false);
    const Vector3& row = Row == 0 ? rotation.x : Row == 1 ? rotation.y : rotation.z;
    return Coordinate(row, Column) * Coordinate(xfm.mScale, Row);
}

// Registers the float members "x", "y" and "z" of a 3D vector struct with
// zero defaults. Inlined into every such struct of the registry. Name not
// in the reference map.
void RegisterVector3Members(PropRegistry& registry) {
    const Vector3 prototype = {0.0F, 0.0F, 0.0F};
    FloatMetadata& x = TypeSpecificMetadata(
        registry.RegisterProp("x", offsetof(Vector3, x), kPropertyFloat, 4), prototype.x);
    x.mDefault = prototype.x;
    FloatMetadata& y = TypeSpecificMetadata(
        registry.RegisterProp("y", offsetof(Vector3, y), kPropertyFloat, 4), prototype.y);
    y.mDefault = prototype.y;
    FloatMetadata& z = TypeSpecificMetadata(
        registry.RegisterProp("z", offsetof(Vector3, z), kPropertyFloat, 4), prototype.z);
    z.mDefault = prototype.z;
}

// Registers a read-only axis of "local_transform": a struct without
// storage whose "x", "y" and "z" read the scaled rotation. Inlined. Name
// not in the reference map.
template <int Row>
void RegisterLocalAxis(PropRegistry& registry, const char* name) {
    GroupMetadata& axis = static_cast<GroupMetadata&>(
        *registry.RegisterProp(name, 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    axis.mKind = 1;
    const float xType = 0.0F;
    FloatMetadata& x = TypeSpecificMetadata(axis.mRegistry.RegisterProp("x", -1, kPropertyFloat, 0), xType);
    static_cast<void>(Symbol("prop"));
    x.mGetter = GetLocalAxisElement<Row, 0>;
    const float yType = 0.0F;
    FloatMetadata& y = TypeSpecificMetadata(axis.mRegistry.RegisterProp("y", -1, kPropertyFloat, 0), yType);
    static_cast<void>(Symbol("prop"));
    y.mGetter = GetLocalAxisElement<Row, 1>;
    const float zType = 0.0F;
    FloatMetadata& z = TypeSpecificMetadata(axis.mRegistry.RegisterProp("z", -1, kPropertyFloat, 0), zType);
    static_cast<void>(Symbol("prop"));
    z.mGetter = GetLocalAxisElement<Row, 2>;
}

// Registers a 3D vector struct member of a transform struct, with zero
// defaults. Inlined. Name not in the reference map.
void RegisterVector3Struct(PropRegistry& registry, const char* name, int offset, bool flagged) {
    GroupMetadata& vector = static_cast<GroupMetadata&>(
        *registry.RegisterProp(name, offset, kPropertyStruct, sizeof(Vector3)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    if (flagged) {
        vector.mKind = 1;
    }
    RegisterVector3Members(vector.mRegistry);
}

// Reads a float property that a revision before 2 saved, such as the
// matrix element "mxx". A missing or unreadable property is reported (the
// message is compiled out) and leaves the value. Inlined into the revision
// handler. Name not in the reference map.
void ReadSavedFloat(Component& component, const char* name, float& value) {
    const PropPath path{Symbol(name)};
    void* storage = &component;
    const PropInfo* const info = component._GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _GetProp(component, path, *info, storage, &value, kPropertyFloat)) {
        return;
    }
    static_cast<void>(component.MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

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

// Inlined into the factory (0x112EF60), the placement constructor
// (0x112EFF0) and the registry (0x1AFFB0).
TransCom::TransCom()
    : mLocalXfm(PropXfm::sIdentity),
      mTransParent{0xFFFFFFFFu},
      mParentDynamic(false),
      mInheritType(0),
      mDirtyFlags(3),
      mUseEntityParent(false),
      mWorldXfm(kIdentityXfm) {}

// Inlined into _Imprint at 0x1B4D74: the properties are copied as bytes.
TransCom::TransCom(const TransCom& other)
    : Component(other),
      mLocalXfm(other.mLocalXfm),
      mTransParent(other.mTransParent),
      mParentDynamic(other.mParentDynamic),
      mInheritType(other.mInheritType),
      mDirtyFlags(3),
      mUseEntityParent(false),
      mWorldXfm(kIdentityXfm) {}

// Reconstructed from eboot.elf at 0x1AFFB0. The legacy "x" to "z_scale"
// floats and the "position", "scale" and "rotation" views share the local
// transform, which is saved once through the floats; the root object's
// transform is read-only and not animatable. "local_transform" and
// "world_transform" are read-only views, and "utilities" holds the "reset"
// and "copy_world_xfm" actions. Revision 2 replaced the saved rotation
// matrix ("mxx" to "mzz"), which the revision handler (0x1B56D0) converts:
// revision 0 into scale and Euler angles, revision 1 into the angles only.
// Every property registration builds a "prop" symbol it does not use, and
// the struct and action registrations "struct" and "action".
void TransCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    TransCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 1) {
                return;
            }
            // The third context pointer is the component the saved
            // properties were loaded into.
            Component& saved = *args.mSavedProps;
            Hmx::Matrix3 rotation = kIdentityRotation;
            ReadSavedFloat(saved, "mxx", rotation.x.x);
            ReadSavedFloat(saved, "mxy", rotation.x.y);
            ReadSavedFloat(saved, "mxz", rotation.x.z);
            ReadSavedFloat(saved, "myx", rotation.y.x);
            ReadSavedFloat(saved, "myy", rotation.y.y);
            ReadSavedFloat(saved, "myz", rotation.y.z);
            ReadSavedFloat(saved, "mzx", rotation.z.x);
            ReadSavedFloat(saved, "mzy", rotation.z.y);
            ReadSavedFloat(saved, "mzz", rotation.z.z);
            TransCom* const trans = static_cast<TransCom*>(args.mComponent);
            if (args.mSavedRev <= 0) {
                trans->mLocalXfm.SetRotation(rotation, false);
            } else {
                // The binary calls MakeEuler's thunk at 0x218610.
                MakeEuler(rotation, trans->mLocalXfm.mEuler);
            }
        };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(2);
        sPropRegistry.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    IntMetadata<int>& inheritType = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "inherit_type", offsetof(TransCom, mInheritType), kPropertyInt, 4).mMetadata.Get());
    inheritType.mDefault = prototype.mInheritType;
    static_cast<void>(Symbol("prop"));
    inheritType.mHelp = String("Which of the parent transform's attributes should this transform inherit?");
    inheritType.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("Entire Transform"), String("Inherit translation, rotation and scale")},
        AllowedValue<int>{1, String("Only Scale and Rotation"), String("Inherit only rotation and scale")},
        AllowedValue<int>{2, String("Only Translation"), String("Inherit only translation")}};
    inheritType.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    inheritType.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<TransCom*>(args.mComponent)->mDirtyFlags |= 0x14;
    };

    const int localXfm = offsetof(TransCom, mLocalXfm);
    FloatMetadata& x = TypeSpecificMetadata(
        registry.RegisterProp("x", localXfm + offsetof(LocalXfm, mPos.x), kPropertyFloat, 4),
        prototype.mLocalXfm.mPos.x);
    x.mDefault = prototype.mLocalXfm.mPos.x;
    static_cast<void>(Symbol("prop"));
    x.mHelp._Set([](const TagArgs& args) { return LocalTransformHelp(args, "Local translation along the X axis"); });
    x.mOnChanged = [](const PropAccessorArgs& args) { static_cast<TransCom*>(args.mComponent)->mDirtyFlags |= 1; };
    x.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    x.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    FloatMetadata& y = TypeSpecificMetadata(
        registry.RegisterProp("y", localXfm + offsetof(LocalXfm, mPos.y), kPropertyFloat, 4),
        prototype.mLocalXfm.mPos.y);
    y.mDefault = prototype.mLocalXfm.mPos.y;
    static_cast<void>(Symbol("prop"));
    y.mHelp._Set([](const TagArgs& args) { return LocalTransformHelp(args, "Local translation along the Y axis"); });
    y.mOnChanged = [](const PropAccessorArgs& args) { static_cast<TransCom*>(args.mComponent)->mDirtyFlags |= 1; };
    y.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    y.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    FloatMetadata& z = TypeSpecificMetadata(
        registry.RegisterProp("z", localXfm + offsetof(LocalXfm, mPos.z), kPropertyFloat, 4),
        prototype.mLocalXfm.mPos.z);
    z.mDefault = prototype.mLocalXfm.mPos.z;
    static_cast<void>(Symbol("prop"));
    z.mHelp._Set([](const TagArgs& args) { return LocalTransformHelp(args, "Local translation along the Z axis"); });
    z.mOnChanged = [](const PropAccessorArgs& args) { static_cast<TransCom*>(args.mComponent)->mDirtyFlags |= 1; };
    z.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    z.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    // The angles are stored in radians and edited in degrees.
    FloatMetadata& xRotate = TypeSpecificMetadata(
        registry.RegisterProp("x_rotate", localXfm + offsetof(LocalXfm, mEuler.x), kPropertyFloat, 4),
        prototype.mLocalXfm.mEuler.x);
    xRotate.mDefault = prototype.mLocalXfm.mEuler.x;
    static_cast<void>(Symbol("prop"));
    xRotate.mDisplayName = String("x rotate (pitch)");
    xRotate.mHelp._Set([](const TagArgs& args) {
        return LocalTransformHelp(args, "Local rotation angle about the X axis (ie. pitch)");
    });
    xRotate.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<TransCom*>(args.mComponent)->mLocalXfm.mEuler.x * 57.29578F;
    };
    xRotate.mSetter = [](const PropAccessorArgs& args) {
        TransCom* const trans = static_cast<TransCom*>(args.mComponent);
        trans->mLocalXfm.mEuler.x = *static_cast<const float*>(args.mValue) * 0.017453292F;
        trans->mDirtyFlags |= 1;
    };
    xRotate.mDefault = 0.0F;
    xRotate.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    xRotate.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    FloatMetadata& yRotate = TypeSpecificMetadata(
        registry.RegisterProp("y_rotate", localXfm + offsetof(LocalXfm, mEuler.y), kPropertyFloat, 4),
        prototype.mLocalXfm.mEuler.y);
    yRotate.mDefault = prototype.mLocalXfm.mEuler.y;
    static_cast<void>(Symbol("prop"));
    yRotate.mDisplayName = String("y rotate (roll)");
    yRotate.mHelp._Set([](const TagArgs& args) {
        return LocalTransformHelp(args, "Local rotation angle about the Y axis (ie. roll)");
    });
    yRotate.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<TransCom*>(args.mComponent)->mLocalXfm.mEuler.y * 57.29578F;
    };
    yRotate.mSetter = [](const PropAccessorArgs& args) {
        TransCom* const trans = static_cast<TransCom*>(args.mComponent);
        trans->mLocalXfm.mEuler.y = *static_cast<const float*>(args.mValue) * 0.017453292F;
        trans->mDirtyFlags |= 1;
    };
    yRotate.mDefault = 0.0F;
    yRotate.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    yRotate.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    FloatMetadata& zRotate = TypeSpecificMetadata(
        registry.RegisterProp("z_rotate", localXfm + offsetof(LocalXfm, mEuler.z), kPropertyFloat, 4),
        prototype.mLocalXfm.mEuler.z);
    zRotate.mDefault = prototype.mLocalXfm.mEuler.z;
    static_cast<void>(Symbol("prop"));
    zRotate.mDisplayName = String("z rotate (yaw)");
    zRotate.mHelp._Set([](const TagArgs& args) {
        return LocalTransformHelp(args, "Local rotation angle about the Z axis (ie. yaw)");
    });
    zRotate.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<TransCom*>(args.mComponent)->mLocalXfm.mEuler.z * 57.29578F;
    };
    zRotate.mSetter = [](const PropAccessorArgs& args) {
        TransCom* const trans = static_cast<TransCom*>(args.mComponent);
        trans->mLocalXfm.mEuler.z = *static_cast<const float*>(args.mValue) * 0.017453292F;
        trans->mDirtyFlags |= 1;
    };
    zRotate.mDefault = 0.0F;
    zRotate.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    zRotate.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    FloatMetadata& xScale = TypeSpecificMetadata(
        registry.RegisterProp("x_scale", localXfm + offsetof(LocalXfm, mScale.x), kPropertyFloat, 4),
        prototype.mLocalXfm.mScale.x);
    xScale.mDefault = prototype.mLocalXfm.mScale.x;
    static_cast<void>(Symbol("prop"));
    xScale.mHelp._Set([](const TagArgs& args) { return LocalTransformHelp(args, "Local scale along the X axis"); });
    xScale.mSetter = [](const PropAccessorArgs& args) {
        TransCom* const trans = static_cast<TransCom*>(args.mComponent);
        trans->mLocalXfm.mScale.x = ClampScale(*static_cast<const float*>(args.mValue));
        trans->mDirtyFlags |= 1;
    };
    xScale.mDefault = 1.0F;
    xScale.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    xScale.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    FloatMetadata& yScale = TypeSpecificMetadata(
        registry.RegisterProp("y_scale", localXfm + offsetof(LocalXfm, mScale.y), kPropertyFloat, 4),
        prototype.mLocalXfm.mScale.y);
    yScale.mDefault = prototype.mLocalXfm.mScale.y;
    static_cast<void>(Symbol("prop"));
    yScale.mHelp._Set([](const TagArgs& args) { return LocalTransformHelp(args, "Local scale along the Y axis"); });
    yScale.mSetter = [](const PropAccessorArgs& args) {
        TransCom* const trans = static_cast<TransCom*>(args.mComponent);
        trans->mLocalXfm.mScale.y = ClampScale(*static_cast<const float*>(args.mValue));
        trans->mDirtyFlags |= 1;
    };
    yScale.mDefault = 1.0F;
    yScale.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    yScale.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    FloatMetadata& zScale = TypeSpecificMetadata(
        registry.RegisterProp("z_scale", localXfm + offsetof(LocalXfm, mScale.z), kPropertyFloat, 4),
        prototype.mLocalXfm.mScale.z);
    zScale.mDefault = prototype.mLocalXfm.mScale.z;
    static_cast<void>(Symbol("prop"));
    zScale.mHelp._Set([](const TagArgs& args) { return LocalTransformHelp(args, "Local scale along the Z axis"); });
    zScale.mSetter = [](const PropAccessorArgs& args) {
        TransCom* const trans = static_cast<TransCom*>(args.mComponent);
        trans->mLocalXfm.mScale.z = ClampScale(*static_cast<const float*>(args.mValue));
        trans->mDirtyFlags |= 1;
    };
    zScale.mDefault = 1.0F;
    zScale.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    zScale.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    // The views PropAnim drives all at once; the editor does not change
    // them and they are not saved.
    Vector3Metadata& position = TypeSpecificMetadata(
        registry.RegisterProp("position", localXfm + offsetof(LocalXfm, mPos), kPropertyVector3, 12),
        prototype.mLocalXfm.mPos);
    position.mDefault = prototype.mLocalXfm.mPos;
    static_cast<void>(Symbol("prop"));
    position.mHelp._Set([](const TagArgs& args) {
        return LocalTransformHelp(
            args, "Local translation in the x, y, and z axis (all at once when driven by PropAnim)");
    });
    position.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<TransCom*>(args.mComponent)->mDirtyFlags |= 1;
    };
    position.mSaves = false;
    position.mReadOnly._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return true;
    });
    position.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    Vector3Metadata& scale = TypeSpecificMetadata(
        registry.RegisterProp("scale", localXfm + offsetof(LocalXfm, mScale), kPropertyVector3, 12),
        prototype.mLocalXfm.mScale);
    scale.mDefault = prototype.mLocalXfm.mScale;
    static_cast<void>(Symbol("prop"));
    scale.mHelp._Set([](const TagArgs& args) {
        return LocalTransformHelp(args, "Local scale in the x, y, and z axis (all at once when driven by PropAnim)");
    });
    scale.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<TransCom*>(args.mComponent)->mDirtyFlags |= 1;
    };
    scale.mSaves = false;
    scale.mReadOnly._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return true;
    });
    scale.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    // The binary reaches the metadata through the Euler angle overload of
    // TypeSpecificMetadata (0x141420).
    EulerMetadata& rotation = static_cast<EulerMetadata&>(*registry.RegisterProp(
        "rotation", localXfm + offsetof(LocalXfm, mEuler), kPropertyEuler, 12).mMetadata.Get());
    rotation.mDefault = prototype.mLocalXfm.mEuler;
    static_cast<void>(Symbol("prop"));
    rotation.mHelp._Set([](const TagArgs& args) {
        return LocalTransformHelp(args, "Local rotation matrix (set all at once when driven by PropAnim)");
    });
    rotation.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<TransCom*>(args.mComponent)->mDirtyFlags |= 1;
    };
    rotation.mSaves = false;
    rotation.mReadOnly._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return true;
    });
    rotation.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });

    // By default the root object; another object of the entity with a
    // TransCom. A parent that changes is animatable only with
    // "parent_dynamic".
    GameObjectIdMetadata& parent = TypeSpecificMetadata(
        registry.RegisterProp("parent", offsetof(TransCom, mTransParent), kPropertyGameObjectId, 4),
        prototype.mTransParent);
    parent.mDefault = prototype.mTransParent;
    static_cast<void>(Symbol("prop"));
    parent.mSetter = [](const PropAccessorArgs& args) {
        TransCom* const trans = static_cast<TransCom*>(args.mComponent);
        trans->mTransParent = *static_cast<const GameObjectId*>(args.mValue);
        trans->mDirtyFlags |= 0x14;
    };
    parent.mRequiredComs = {sId};
    parent.mAllowsThis = false;
    parent.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    parent.mAllowedObjects._Set([](const TagArgs& args) {
        eastl::vector<GameObjectId> parents;
        GetValidTransParentsFromObject(args.mComponent.mObject, parents, true);
        return parents;
    });
    parent.mDefault._Set([](const TagArgs& args) {
        const GameObject* const root = args.mComponent.mObject->mEntity->GetRoot();
        if (root == args.mComponent.mObject) {
            return gNullObjectId;
        }
        // The binary also keeps the null id when that id is zero.
        return root != nullptr && gNullObjectId.mId != 0 ? root->mId : gNullObjectId;
    });
    parent.mAnimatable._Set([](const TagArgs& args) {
        if (IsRootObject(args.mComponent)) {
            return false;
        }
        return static_cast<const TransCom&>(args.mComponent).mParentDynamic;
    });

    BoolMetadata& parentDynamic = TypeSpecificMetadata(
        registry.RegisterProp("parent_dynamic", offsetof(TransCom, mParentDynamic), kPropertyBool, 1),
        prototype.mParentDynamic);
    parentDynamic.mDefault = prototype.mParentDynamic;
    static_cast<void>(Symbol("prop"));
    parentDynamic.mHelp = String("Does my parent ever change frame-to-frame? (eg. enables animation by stategraph)");
    parentDynamic.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    parentDynamic.mAnimatable = false;

    GroupMetadata& local = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "local_transform", offsetof(TransCom, mLocalXfm), kPropertyStruct, sizeof(LocalXfm)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    local.mReadOnly = true;
    local.mSaves = false;
    RegisterVector3Struct(local.mRegistry, "position", offsetof(LocalXfm, mPos), true);
    RegisterLocalAxis<0>(local.mRegistry, "x_axis");
    RegisterLocalAxis<1>(local.mRegistry, "y_axis");
    RegisterLocalAxis<2>(local.mRegistry, "z_axis");

    GroupMetadata& world = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "world_transform", offsetof(TransCom, mWorldXfm), kPropertyStruct, sizeof(Transform)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    world.mReadOnly = true;
    world.mSaves = false;
    RegisterVector3Struct(world.mRegistry, "position", offsetof(Transform, v), true);
    RegisterVector3Struct(world.mRegistry, "x_axis", offsetof(Transform, m.x), true);
    RegisterVector3Struct(world.mRegistry, "y_axis", offsetof(Transform, m.y), true);
    RegisterVector3Struct(world.mRegistry, "z_axis", offsetof(Transform, m.z), true);

    PropRegistry& utilities = static_cast<GroupMetadata&>(
        *registry.RegisterProp("utilities", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    ActionMetadata& reset = static_cast<ActionMetadata&>(*utilities.RegisterAction("reset").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    reset.mHelp = String("Reset my local transform to the identity matrix.");
    reset.mAction = [](const TagArgs& args) {
        TransCom& trans = const_cast<TransCom&>(static_cast<const TransCom&>(args.mComponent));
        trans.mLocalXfm = PropXfm::sIdentity;
        trans.mDirtyFlags |= 1;
    };

    GroupMetadata& copyWorldXfm = static_cast<GroupMetadata&>(
        *utilities.RegisterProp("copy_world_xfm", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    copyWorldXfm.mHelp._Set([](const TagArgs& args) {
        return LocalTransformHelp(
            args, "Set my local transform so that my world transform matches another TransCom's world transform");
    });
    copyWorldXfm.mReadOnly._Set([](const TagArgs& args) { return IsRootObject(args.mComponent); });
    copyWorldXfm.mAnimatable._Set([](const TagArgs& args) { return !IsRootObject(args.mComponent); });
    const GameObjectId fromType = {0xFFFFFFFFu};
    GameObjectIdMetadata& from =
        TypeSpecificMetadata(copyWorldXfm.mRegistry.RegisterProp("from", -1, kPropertyGameObjectId, 0), fromType);
    static_cast<void>(Symbol("prop"));
    from.mGetter = [](const PropAccessorArgs& args) {
        static_cast<void>(args);
        return gCopyWorldXfmSource;
    };
    from.mSetter = [](const PropAccessorArgs& args) {
        gCopyWorldXfmSource = *static_cast<const GameObjectId*>(args.mValue);
    };
    ActionMetadata& copy =
        static_cast<ActionMetadata&>(*copyWorldXfm.mRegistry.RegisterAction("copy_world_xfm").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    copy.mAction = [](const TagArgs& args) {
        if (gCopyWorldXfmSource.mId == gNullObjectId.mId) {
            return;
        }
        TransCom& trans = const_cast<TransCom&>(static_cast<const TransCom&>(args.mComponent));
        const GameObject* const source = trans.mObject->mEntity->GetObject(gCopyWorldXfmSource);
        if (source == nullptr) {
            return;
        }
        const TransCom* const sourceTrans = source->GetCom<TransCom>();
        if (sourceTrans == nullptr) {
            return;
        }
        trans.mLocalXfm = TransUtl::CalcNewLocalXfm(trans, sourceTrans->mWorldXfm);
        trans.mDirtyFlags |= 1;
    };
    copy.mReadOnly._Set([](const TagArgs& args) {
        if (gCopyWorldXfmSource.mId == gNullObjectId.mId) {
            return true;
        }
        const GameObject* const source = args.mComponent.mObject->mEntity->GetObject(gCopyWorldXfmSource);
        return source == nullptr || source->GetCom<TransCom>() == nullptr;
    });

    metadata.mDescription = "The transform of your object.";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryMiscellaneous;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());
    metadata.mLightweight = true;
    metadata.mEditorRestrictions = 1;
    metadata.mAliases.push_back(Symbol("TransCom"));
}

// Reconstructed from eboot.elf at 0x1B3A90.
void TransCom::SetTransParent(GameObjectId parent, bool keepWorld) {
    mTransParent = parent;
    mDirtyFlags |= keepWorld ? 0x14 : 0x4;
}

// Reconstructed from eboot.elf at 0x1B4010. The report of the error is
// compiled out.
bool TransCom::_OnResourcesLoaded() {
    if (mTransParent.mId == mObject->mId.mId) {
        static_cast<void>(MakeErrorName());
        mTransParent = gNullObjectId;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x1B46E0.
void TransCom::_GetPollDeps(eastl::vector<GameObjectId>& before, eastl::vector<GameObjectId>& after) {
    static_cast<void>(after);
    if (mTransParent.mId != gNullObjectId.mId) {
        before.push_back(mTransParent);
    }
}

// Reconstructed from eboot.elf at 0x1B47C0.
void TransCom::_OnActivate() {
    mDirtyFlags |= 1;
}

// Reconstructed from eboot.elf at 0x1B47D0.
void TransCom::_EditEnter() {
    TransCom::_Enter();
}

// Reconstructed from eboot.elf at 0x1B47E0.
void TransCom::_EditPoll() {
    TransCom::_Poll();
}

// Reconstructed from eboot.elf at 0x1B3A80. The deleting destructor is at
// 0x1B4C90.
TransCom::~TransCom() {}

// Reconstructed from eboot.elf at 0x1B4CB0.
Symbol TransCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x1B4CC0.
Symbol TransCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x1B4CD0.
int TransCom::CurrentRev() const {
    return const_cast<TransCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x1B4CF0.
bool TransCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x1B4D20.
Component* TransCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x1B4D30. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* TransCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<TransCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) TransCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x1B4EA0.
PropRegistry& TransCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x1B4EB0.
ComMetaData& TransCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x112EF60, emitted in entity/Utl.o with
// the class's Init.
Component* TransCom::_Create() {
    return new TransCom();
}

// Reconstructed from eboot.elf at 0x112D830, in entity/Utl.o.
void TransCom::Init() {
    InitComponentClass<TransCom>();
}
