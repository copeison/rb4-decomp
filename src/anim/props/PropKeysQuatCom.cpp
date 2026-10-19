// anim/PropKeysQuatCom.o (0x311080-0x31498F). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysQuatCom.h"

#include <math.h>
#include <new>

#include "anim/props/PropAnimResource.h"
#include "entity/core/TransCom.h"
#include "math/matrix/Matrix3.h"
#include "math/rotation/Rot.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataUtl.h"

namespace {

// Degrees per radian and back, as the editor shows the angles.
constexpr float kRadToDeg = 57.29578f;
constexpr float kDegToRad = 0.017453292f;
constexpr float kPi = 3.1415927f;

// The key whose Euler angle an accessor reads or writes: the path is
// "keys", the index, "value", "euler_rotation" and the angle. Name not in
// the reference map.
Hmx::Quat& KeyQuat(const PropAccessorArgs& args) {
    PropKeysQuatCom* const keys = static_cast<PropKeysQuatCom*>(args.mComponent);
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[1].mValue;
    return static_cast<TKey<Hmx::Quat>*>(keys->mKeys.mData)[index].mValue;
}

// The key's Euler angles in degrees. Inlined into the getters. Name not in
// the reference map.
Vector3 KeyEulerDegrees(const PropAccessorArgs& args) {
    Vector3 angles = {0.0f, 0.0f, 0.0f};
    MakeEuler(KeyQuat(args), angles);
    return Vector3{angles.x * kRadToDeg, angles.y * kRadToDeg, angles.z * kRadToDeg};
}

// The angle wrapped into [-pi, pi). Inlined into the setters. Name not in
// the reference map.
float WrapAngle(float angle) {
    const float wrapped = fmodf(angle + kPi, kPi * 2.0f);
    return wrapped < 0.0f ? wrapped + kPi : wrapped - kPi;
}

// Adds the quaternion, weighted, to the sum: x, y and z scale by the
// weight's magnitude and w by the weight, and the result is subtracted
// instead when it lies on the other hemisphere from the sum. Inlined into
// the driver. Name not in the reference map.
void AccumulateQuat(Hmx::Quat& sum, const Hmx::Quat& quat, float weight) {
    const float scale = fabsf(weight);
    const Hmx::Quat scaled = {quat.x * scale, quat.y * scale, quat.z * scale, quat.w * weight};
    if (sum.x * scaled.x + sum.y * scaled.y + sum.z * scaled.z + sum.w * scaled.w >= 0.0f) {
        sum.x += scaled.x;
        sum.y += scaled.y;
        sum.z += scaled.z;
        sum.w += scaled.w;
    } else {
        sum.x -= scaled.x;
        sum.y -= scaled.y;
        sum.z -= scaled.z;
        sum.w -= scaled.w;
    }
}

// The identity rotation the matrix writes start from. Name not in the
// reference map.
constexpr Hmx::Matrix3 kIdentityRotation = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

static_assert(offsetof(TransCom, mDirtyFlags) == 72);

}  // namespace

// The object's statics, in the order of its static initializer (0x314820).
Symbol PropKeysQuatCom::sId("PropKeysQuatCom");
Symbol PropKeysQuatCom::sClassName("PropKeysQuatCom");
PropRegistry PropKeysQuatCom::sPropRegistry;
ComMetaData PropKeysQuatCom::sMetaData;

// Reconstructed from eboot.elf at 0x311080.
PropDriver* PropKeysQuatCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysQuatCom>(prop);
}

// Reconstructed from eboot.elf at 0x311150.
void PropKeysQuatCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x311230.
PropDriver* PropKeysQuatCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysQuatCom>(mProp);
}

// Reconstructed from eboot.elf at 0x311300.
const char* PropKeysQuatCom::StaticKeyTypeName() {
    return "TKey<kStruct>";
}

// Reconstructed from eboot.elf at 0x311310. The struct registration is
// RegisterPropStruct<TKey<Hmx::Quat>> (0x311B20) and the base properties
// _InitPropKeysBase<PropKeysQuatCom> (0x311CD0). The key's "value" is a
// struct of the Euler angles in degrees, read and written through the
// accessors, and "quat_rotation" the hidden stored quaternion.
void PropKeysQuatCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<Hmx::Quat>>(Symbol("TKey<Quat>"));
    PropKeysQuatCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysQuatCom, mKeys), sizeof(PropArray<TKey<Hmx::Quat>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<Hmx::Quat>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<Hmx::Quat> keyPrototype = {0.0f, {0.0f, 0.0f, 0.0f, 1.0f}};
    key.mHelp = String("A Quaternion rotation Key in the PropAnim!");
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<Hmx::Quat>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    PropRegistry& value =
        static_cast<GroupMetadata&>(*key.mRegistry.RegisterProp("value", 0, kPropertyStruct, 0).mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct"));
    GroupMetadata& euler =
        static_cast<GroupMetadata&>(*value.RegisterProp("euler_rotation", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    euler.mKind = 1;

    const float pitchType = 0.0f;
    FloatMetadata& pitch = TypeSpecificMetadata(euler.mRegistry.RegisterProp("x", -1, kPropertyFloat, 0), pitchType);
    static_cast<void>(Symbol("prop"));
    pitch.mHelp = String("Rotation angle about the X axis (ie. pitch)");
    // At 0x313BF0 and 0x313C60.
    pitch.mGetter = [](const PropAccessorArgs& args) { return KeyEulerDegrees(args).x; };
    pitch.mSetter = [](const PropAccessorArgs& args) {
        Hmx::Quat& quat = KeyQuat(args);
        Vector3 angles = {0.0f, 0.0f, 0.0f};
        MakeEuler(quat, angles);
        angles.x = WrapAngle(*static_cast<const float*>(args.mValue) * kDegToRad);
        quat.Set(angles);
    };
    const float rollType = 0.0f;
    FloatMetadata& roll = TypeSpecificMetadata(euler.mRegistry.RegisterProp("y", -1, kPropertyFloat, 0), rollType);
    static_cast<void>(Symbol("prop"));
    roll.mHelp = String("Rotation angle about the Y axis (ie. roll)");
    // At 0x313D30 and 0x313DA0.
    roll.mGetter = [](const PropAccessorArgs& args) { return KeyEulerDegrees(args).y; };
    roll.mSetter = [](const PropAccessorArgs& args) {
        Hmx::Quat& quat = KeyQuat(args);
        Vector3 angles = {0.0f, 0.0f, 0.0f};
        MakeEuler(quat, angles);
        angles.y = WrapAngle(*static_cast<const float*>(args.mValue) * kDegToRad);
        quat.Set(angles);
    };
    const float yawType = 0.0f;
    FloatMetadata& yaw = TypeSpecificMetadata(euler.mRegistry.RegisterProp("z", -1, kPropertyFloat, 0), yawType);
    static_cast<void>(Symbol("prop"));
    yaw.mHelp = String("Rotation angle about the Z axis (ie. yaw)");
    // At 0x313E70 and 0x313EE0.
    yaw.mGetter = [](const PropAccessorArgs& args) { return KeyEulerDegrees(args).z; };
    yaw.mSetter = [](const PropAccessorArgs& args) {
        Hmx::Quat& quat = KeyQuat(args);
        Vector3 angles = {0.0f, 0.0f, 0.0f};
        MakeEuler(quat, angles);
        angles.z = WrapAngle(*static_cast<const float*>(args.mValue) * kDegToRad);
        quat.Set(angles);
    };

    PropRegistry& quat = static_cast<GroupMetadata&>(
                             *key.mRegistry
                                  .RegisterProp(
                                      "quat_rotation", offsetof(TKey<Hmx::Quat>, mValue), kPropertyStruct,
                                      sizeof(Hmx::Quat))
                                  .mMetadata.Get())
                             .mRegistry;
    static_cast<void>(Symbol("struct"));
    const Hmx::Quat quatPrototype = {0.0f, 0.0f, 0.0f, 1.0f};
    FloatMetadata& x =
        TypeSpecificMetadata(quat.RegisterProp("x", offsetof(Hmx::Quat, x), kPropertyFloat, 4), quatPrototype.x);
    x.mDefault = quatPrototype.x;
    static_cast<void>(Symbol("prop"));
    x.mIsHidden = true;
    FloatMetadata& y =
        TypeSpecificMetadata(quat.RegisterProp("y", offsetof(Hmx::Quat, y), kPropertyFloat, 4), quatPrototype.y);
    y.mDefault = quatPrototype.y;
    static_cast<void>(Symbol("prop"));
    y.mIsHidden = true;
    FloatMetadata& z =
        TypeSpecificMetadata(quat.RegisterProp("z", offsetof(Hmx::Quat, z), kPropertyFloat, 4), quatPrototype.z);
    z.mDefault = quatPrototype.z;
    static_cast<void>(Symbol("prop"));
    z.mIsHidden = true;
    FloatMetadata& w =
        TypeSpecificMetadata(quat.RegisterProp("w", offsetof(Hmx::Quat, w), kPropertyFloat, 4), quatPrototype.w);
    w.mDefault = quatPrototype.w;
    static_cast<void>(Symbol("prop"));
    w.mIsHidden = true;
    _InitPropKeysBase<PropKeysQuatCom>(registry);
    metadata.mDescription = "Com that contains the Quaternion rotation PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x312AA0.
Symbol PropKeysQuatCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x312AB0.
Symbol PropKeysQuatCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x312AC0.
int PropKeysQuatCom::CurrentRev() const {
    return const_cast<PropKeysQuatCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x312AE0.
bool PropKeysQuatCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x312B10.
Component* PropKeysQuatCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x312B20. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysQuatCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysQuatCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysQuatCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x312D50.
PropRegistry& PropKeysQuatCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x312D60.
ComMetaData& PropKeysQuatCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x312FB0.
PropertyType PropKeysQuatCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x312FC0.
Symbol PropKeysQuatCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x313060.
bool PropKeysQuatCom::IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
    static_cast<void>(interpolation);
    return true;
}

// Reconstructed from eboot.elf at 0x3133B0.
DataNode PropKeysQuatCom::_ValueToDataNode(const Hmx::Quat& value, bool forEditor) const {
    if (forEditor) {
        Vector3 angles = {0.0f, 0.0f, 0.0f};
        MakeEuler(value, angles);
        angles.x *= kRadToDeg;
        angles.y *= kRadToDeg;
        angles.z *= kRadToDeg;
        return DataNode(MakeDataArray(DataNode(angles.x), DataNode(angles.y), DataNode(angles.z)), kDataArray);
    }
    return DataNode(
        MakeDataArray(DataNode(value.x), DataNode(value.y), DataNode(value.z), DataNode(value.w)), kDataArray);
}

// Reconstructed from eboot.elf at 0x313740.
Hmx::Quat PropKeysQuatCom::_ValueFromDataNode(const DataNode& value) const {
    Hmx::Quat quat = {0.0f, 0.0f, 0.0f, 1.0f};
    const DataArrayPtr array = value.Array(nullptr);
    if (array->Size() == 3) {
        const float x = array->Float(0);
        const float y = array->Float(1);
        const float z = array->Float(2);
        const Vector3 angles = {x * kDegToRad, y * kDegToRad, z * kDegToRad};
        Hmx::Quat rotation;
        rotation.Set(angles);
        Normalize(rotation, quat);
    } else {
        const float x = array->Float(0);
        const float y = array->Float(1);
        const float z = array->Float(2);
        const float w = array->Float(3);
        quat = Hmx::Quat{x, y, z, w};
    }
    return quat;
}

// Reconstructed from eboot.elf at 0x3138B0.
Hmx::Quat PropKeysQuatCom::_DefaultValue() const {
    return Hmx::Quat{0.0f, 0.0f, 0.0f, 0.0f};
}

// Reconstructed from eboot.elf at 0x312890. The property's storage holds
// the rotation matrix.
template <>
void PropKeysTypeCom<Hmx::Quat>::UpdateValueAtIndex(unsigned long index, Entity* entity) {
    Component* const component = mProp.Com(entity);
    if (component != nullptr) {
        void* storage;
        mProp.Find(component, &storage);
        KeyAt(index).mValue.Set(*static_cast<const Hmx::Matrix3*>(storage));
    }
}

// Reconstructed from eboot.elf at 0x3127D0. Without the component the key
// takes the default value.
template <>
unsigned long PropKeysTypeCom<Hmx::Quat>::AddNewKey(float frame, Entity* entity, bool replace) {
    Component* const component = mProp.Com(entity);
    Hmx::Quat value = _DefaultValue();
    if (component != nullptr) {
        void* storage;
        mProp.Find(component, &storage);
        value.Set(*static_cast<const Hmx::Matrix3*>(storage));
    }
    return this->AddKey(value, frame, replace, false);
}

// Reconstructed from eboot.elf at 0x312910.
template <>
bool PropKeysTypeCom<Hmx::Quat>::ApplyFrame(float frame, Entity* entity) const {
    Component* const component = mProp.Com(entity);
    if (component == nullptr) {
        return false;
    }
    void* storage;
    mProp.Find(component, &storage);
    Hmx::Quat value = {0.0f, 0.0f, 0.0f, 1.0f};
    this->SampleValueAtFrame(frame, value);
    Hmx::Matrix3 rotation = kIdentityRotation;
    MakeRotMatrix(value, rotation);
    *static_cast<Hmx::Matrix3*>(storage) = rotation;
    static_cast<TransCom*>(component)->mDirtyFlags |= 1;
    return true;
}

// Reconstructed from eboot.elf at 0x312270. The first sample is scaled by
// its weight and starts the sum; the final sum is accumulated with itself
// scaled by the inverse of the weights before it is normalized.
template <>
void TPropDriver<PropKeysQuatCom>::_Poll(GameObject* root) {
    static_cast<void>(root);
    Hmx::Quat sum = {0.0f, 0.0f, 0.0f, 1.0f};
    Hmx::Quat sample = {0.0f, 0.0f, 0.0f, 1.0f};
    float weights = 0.0f;
    bool first = true;
    for (const Entry* entry = mEntries.mpBegin; entry != mEntries.mpEnd; ++entry) {
        if (entry->mInfo->mPaused) {
            continue;
        }
        entry->mKeys->SampleValueAtFrame(entry->mInfo->GetCurrentFrame(), sample);
        const float weight = entry->mInfo->mWeight;
        if (first) {
            sample = Hmx::Quat{sample.x * weight, sample.y * weight, sample.z * weight, sample.w * weight};
            sum = sample;
            weights = weight;
            first = false;
        } else {
            AccumulateQuat(sum, sample, weight);
            weights += weight;
        }
    }
    if (weights > 0.0f && sum.x * sum.x + sum.y * sum.y + sum.z * sum.z + sum.w * sum.w != 0.0f) {
        const Hmx::Quat total = sum;
        AccumulateQuat(sum, total, 1.0f / weights);
        Normalize(sum, sum);
        if (mEntries.mpBegin->mCom != nullptr) {
            _SetDrivenProperty(mEntries.mpBegin->mCom, mEntries.mpBegin->mKeys->mProp.mPath, &sum);
        }
    }
}

// Reconstructed from eboot.elf at 0x312720.
template <>
void TPropDriver<PropKeysQuatCom>::_SetDrivenProperty(Component* component, const PropPath& path, const void* value) {
    void* storage = component;
    component->_GetPropRegistry().FindProp(path, &storage);
    Hmx::Matrix3 rotation = kIdentityRotation;
    MakeRotMatrix(*static_cast<const Hmx::Quat*>(value), rotation);
    *static_cast<Hmx::Matrix3*>(storage) = rotation;
    static_cast<TransCom*>(component)->mDirtyFlags |= 1;
}

// Reconstructed from eboot.elf at 0x2B3CC0. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysQuatCom::_Create() {
    return new PropKeysQuatCom();
}

// Reconstructed from eboot.elf at 0x2ABC40. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysQuatCom::Init() {
    InitComponentClass<PropKeysQuatCom>();
}
