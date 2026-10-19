// anim/PropKeysRotationCom.o (0x314990-0x318DAF). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysRotationCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"
#include "entity/props/PropUtl.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataUtl.h"

namespace {

// The key whose value an accessor reads or writes: the path ends in the
// key's index and "value". Name not in the reference map.
template <class T>
T& AccessedKey(const PropAccessorArgs& args) {
    PropKeysTypeCom<T>* const keys = static_cast<PropKeysTypeCom<T>*>(args.mComponent);
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-2].mValue;
    return static_cast<TKey<T>*>(keys->mKeys.mData)[index].mValue;
}

// The float the key's struct saved as "<value> <angle>" before revision 1,
// or 0. Inlined into the revision handler. Name not in the reference map.
float ReadSavedAngle(const PropRegistry::RevisionArgs& args, Symbol value, Symbol angle) {
    PropPath path = *args.mPath;
    PropPath::Node node;
    path.PushNode(node = value);
    path.PushNode(node = angle);
    float result = 0.0f;
    Component& saved = *args.mSavedProps;
    void* storage = &saved;
    const PropInfo* const info = saved._GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _GetProp(saved, path, *info, storage, &result, kPropertyFloat)) {
        return result;
    }
    static_cast<void>(saved.MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
    return result;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x318C40).
Symbol PropKeysRotationCom::sId("PropKeysRotationCom");
Symbol PropKeysRotationCom::sClassName("PropKeysRotationCom");
PropRegistry PropKeysRotationCom::sPropRegistry;
ComMetaData PropKeysRotationCom::sMetaData;

// Reconstructed from eboot.elf at 0x314990.
PropDriver* PropKeysRotationCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysRotationCom>(prop);
}

// Reconstructed from eboot.elf at 0x314A60.
void PropKeysRotationCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x314B40.
PropDriver* PropKeysRotationCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysRotationCom>(mProp);
}

// Reconstructed from eboot.elf at 0x314C10.
const char* PropKeysRotationCom::StaticKeyTypeName() {
    return "TKey<kRotation>";
}

// Reconstructed from eboot.elf at 0x314C20.
PropKeysRotationCom::PropKeysRotationCom() {
    mInterpolation = static_cast<KeyTrackBase::Interpolation>(KeyTrackBase::kInterpolationLinear | 0x40000000);
}

// Reconstructed from eboot.elf at 0x314CD0. The struct registration is
// RegisterPropStruct<TKey<RotationKey>> (0x315420) and the base properties
// _InitPropKeysBase<PropKeysRotationCom> (0x3155D0). The key's "value" is
// the Euler angles, read and written through the accessors; the key itself
// is saved as "serialized_value". The key struct's registry is at revision
// 1, whose handler reads the angles saved before it.
void PropKeysRotationCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<RotationKey>>(Symbol("TKey<RotationKey>"));
    PropKeysRotationCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysRotationCom, mKeys), sizeof(PropArray<TKey<RotationKey>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<RotationKey>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<RotationKey> keyPrototype = {0.0f, RotationKey()};
    key.mHelp = String("A Rotation Key in the PropAnim!");
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<RotationKey>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    const PropRotation valueType = PropRotation();
    EulerMetadata& value = TypeSpecificMetadata(key.mRegistry.RegisterProp("value", -1, kPropertyEuler, 0), valueType);
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    // At 0x317560 and 0x3175E0.
    value.mGetter = [](const PropAccessorArgs& args) {
        const RotationKey& rotation = AccessedKey<RotationKey>(args);
        rotation.UpdateEuler();
        return rotation.mEuler;
    };
    value.mSetter = [](const PropAccessorArgs& args) {
        RotationKey& rotation = AccessedKey<RotationKey>(args);
        rotation.mEuler = *static_cast<const Vector3*>(args.mValue);
        rotation.mFlags &= ~RotationKey::kEulerStale;
        rotation.mQuat.Set(rotation.mEuler);
        rotation.mFlags &= ~RotationKey::kQuatStale;
    };

    PropRegistry& serialized = static_cast<GroupMetadata&>(
                                   *key.mRegistry
                                        .RegisterProp(
                                            "serialized_value", offsetof(TKey<RotationKey>, mValue), kPropertyStruct,
                                            sizeof(RotationKey))
                                        .mMetadata.Get())
                                   .mRegistry;
    static_cast<void>(Symbol("struct"));
    const RotationKey serializedPrototype;
    EulerMetadata& euler = TypeSpecificMetadata(
        serialized.RegisterProp("euler", offsetof(RotationKey, mEuler), kPropertyEuler, sizeof(Vector3)), valueType);
    euler.mDefault = serializedPrototype.mEuler;
    PropRegistry& quat = static_cast<GroupMetadata&>(
                             *serialized.RegisterProp("quat", offsetof(RotationKey, mQuat), kPropertyStruct, sizeof(Hmx::Quat))
                                  .mMetadata.Get())
                             .mRegistry;
    static_cast<void>(Symbol("struct"));
    const Hmx::Quat quatPrototype = {0.0f, 0.0f, 0.0f, 1.0f};
    FloatMetadata& x =
        TypeSpecificMetadata(quat.RegisterProp("x", offsetof(Hmx::Quat, x), kPropertyFloat, 4), quatPrototype.x);
    x.mDefault = quatPrototype.x;
    FloatMetadata& y =
        TypeSpecificMetadata(quat.RegisterProp("y", offsetof(Hmx::Quat, y), kPropertyFloat, 4), quatPrototype.y);
    y.mDefault = quatPrototype.y;
    FloatMetadata& z =
        TypeSpecificMetadata(quat.RegisterProp("z", offsetof(Hmx::Quat, z), kPropertyFloat, 4), quatPrototype.z);
    z.mDefault = quatPrototype.z;
    FloatMetadata& w =
        TypeSpecificMetadata(quat.RegisterProp("w", offsetof(Hmx::Quat, w), kPropertyFloat, 4), quatPrototype.w);
    w.mDefault = quatPrototype.w;
    const unsigned char flagsPrototype = serializedPrototype.mFlags;
    IntMetadata<unsigned char>& flags = TypeSpecificMetadata(
        serialized.RegisterProp("flags", offsetof(RotationKey, mFlags), kPropertyUInt8, 1), flagsPrototype);
    flags.mDefault = flagsPrototype;

    // At 0x317C40: keys saved before revision 1 kept their angles as the
    // floats "value pitch", "value roll" and "value yaw".
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev > 0) {
            return;
        }
        static Symbol valueName;
        if (valueName == Symbol()) {
            valueName = Symbol("value");
        }
        static Symbol pitchName;
        if (pitchName == Symbol()) {
            pitchName = Symbol("pitch");
        }
        static Symbol rollName;
        if (rollName == Symbol()) {
            rollName = Symbol("roll");
        }
        static Symbol yawName;
        if (yawName == Symbol()) {
            yawName = Symbol("yaw");
        }
        const float pitch = ReadSavedAngle(args, valueName, pitchName);
        const float roll = ReadSavedAngle(args, valueName, rollName);
        const float yaw = ReadSavedAngle(args, valueName, yawName);
        PropKeysRotationCom* const keys = static_cast<PropKeysRotationCom*>(args.mComponent);
        const unsigned long index = (*args.mPath)[-1].mValue;
        RotationKey& rotation = static_cast<TKey<RotationKey>*>(keys->mKeys.mData)[index].mValue;
        rotation.mEuler = Vector3{pitch, roll, yaw};
        rotation.mFlags &= ~RotationKey::kEulerStale;
        rotation.mQuat.Set(rotation.mEuler);
        rotation.mFlags &= ~RotationKey::kQuatStale;
    };
    key.mRegistry.SetCurrentRev(1);
    key.mRegistry.mRevisionFunc = onRevision;
    _InitPropKeysBase<PropKeysRotationCom>(registry);
    metadata.mDescription = "Com that contains the Rotation PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x316500.
Symbol PropKeysRotationCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x316510.
Symbol PropKeysRotationCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x316520.
int PropKeysRotationCom::CurrentRev() const {
    return const_cast<PropKeysRotationCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x316540.
bool PropKeysRotationCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x316570.
Component* PropKeysRotationCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x316580. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysRotationCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysRotationCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysRotationCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x3167B0.
PropRegistry& PropKeysRotationCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x3167C0.
ComMetaData& PropKeysRotationCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x316A10.
PropertyType PropKeysRotationCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x316A20.
Symbol PropKeysRotationCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x316AC0.
bool PropKeysRotationCom::IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
    static_cast<void>(interpolation);
    return true;
}

// Reconstructed from eboot.elf at 0x316AD0.
bool PropKeysRotationCom::IsRotationKeys() const {
    return true;
}

// Reconstructed from eboot.elf at 0x316AE0.
bool PropKeysRotationCom::HasSerializedValue() const {
    return true;
}

// Reconstructed from eboot.elf at 0x316E90.
DataNode PropKeysRotationCom::_ValueToDataNode(const RotationKey& value, bool forEditor) const {
    static_cast<void>(forEditor);
    value.UpdateEuler();
    return DataNode(
        MakeDataArray(DataNode(value.mEuler.x), DataNode(value.mEuler.y), DataNode(value.mEuler.z)), kDataArray);
}

// Reconstructed from eboot.elf at 0x317020. The node's array is read once
// and dropped before the three reads.
RotationKey PropKeysRotationCom::_ValueFromDataNode(const DataNode& value) const {
    static_cast<void>(value.Array(nullptr));
    return RotationKey(
        Vector3{value.Array(nullptr)->Float(0), value.Array(nullptr)->Float(1), value.Array(nullptr)->Float(2)});
}

// Reconstructed from eboot.elf at 0x3171E0.
RotationKey PropKeysRotationCom::_DefaultValue() const {
    return RotationKey();
}

// Reconstructed from eboot.elf at 0x315CC0.
template <>
void PropKeysTypeCom<RotationKey>::UpdateValueAtIndex(unsigned long index, Entity* entity) {
    KeyAt(index).mValue = RotationKey(mProp.Get<PropRotation>(entity));
}

// Reconstructed from eboot.elf at 0x315B70.
template <>
unsigned long PropKeysTypeCom<RotationKey>::AddNewKey(float frame, Entity* entity, bool replace) {
    return this->AddKey(RotationKey(mProp.Get<PropRotation>(entity)), frame, replace, false);
}

// Reconstructed from eboot.elf at 0x315E00. The sample's Euler angles are
// refreshed before the property is set.
template <>
bool PropKeysTypeCom<RotationKey>::ApplyFrame(float frame, Entity* entity) const {
    RotationKey value;
    this->SampleValueAtFrame(frame, value);
    value.UpdateEuler();
    GameObject* const object = mProp.Obj(entity);
    if (object == nullptr) {
        return false;
    }
    Component* const component = object->FindCom(mProp.mCom);
    if (component == nullptr) {
        return false;
    }
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(mProp.mPath, &storage);
    if (info != nullptr && _SetProp(*component, mProp.mPath, *info, storage, &value.mEuler, kPropertyEuler, false)) {
        return true;
    }
    static_cast<void>(component->MakeErrorName());
    char path[512] = {};
    mProp.mPath.ToString(path, sizeof(path));
    return false;
}

// Reconstructed from eboot.elf at 0x316250. The binary takes the keys'
// PropRef.
template <>
void TPropDriver<PropKeysRotationCom>::_SetDrivenProperty(Component* component, const PropPath& path, const void* value) {
    const RotationKey& key = *static_cast<const RotationKey*>(value);
    key.UpdateEuler();
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _SetProp(*component, path, *info, storage, &key.mEuler, kPropertyEuler, false)) {
        return;
    }
    static_cast<void>(component->MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

// Reconstructed from eboot.elf at 0x2B2C70. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysRotationCom::_Create() {
    return new PropKeysRotationCom();
}

// Reconstructed from eboot.elf at 0x2AB3C0. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysRotationCom::Init() {
    InitComponentClass<PropKeysRotationCom>();
}
