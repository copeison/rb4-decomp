// anim/PropKeysTransformSRTCom.o (0x31E2E0-0x32233F). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysTransformSRTCom.h"

#include <new>

#include "anim/props/PropAnimResource.h"
#include "utl/data/DataArray.h"

namespace {

// The key whose value an accessor reads or writes: the path ends in the
// key's index and "value". Name not in the reference map.
template <class T>
T& AccessedKey(const PropAccessorArgs& args) {
    PropKeysTypeCom<T>* const keys = static_cast<PropKeysTypeCom<T>*>(args.mComponent);
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-2].mValue;
    return static_cast<TKey<T>*>(keys->mKeys.mData)[index].mValue;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x3221D0).
Symbol PropKeysTransformSRTCom::sId("PropKeysTransformSRTCom");
Symbol PropKeysTransformSRTCom::sClassName("PropKeysTransformSRTCom");
PropRegistry PropKeysTransformSRTCom::sPropRegistry;
ComMetaData PropKeysTransformSRTCom::sMetaData;

// Reconstructed from eboot.elf at 0x31E2E0.
PropDriver* PropKeysTransformSRTCom::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysTransformSRTCom>(prop);
}

// Reconstructed from eboot.elf at 0x31E3B0.
void PropKeysTransformSRTCom::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x31E490.
PropDriver* PropKeysTransformSRTCom::CreatePropDriver() const {
    return new TPropDriver<PropKeysTransformSRTCom>(mProp);
}

// Reconstructed from eboot.elf at 0x31E560.
const char* PropKeysTransformSRTCom::StaticKeyTypeName() {
    return "TKey<kTransformSRT>";
}

// Reconstructed from eboot.elf at 0x31E570.
PropKeysTransformSRTCom::PropKeysTransformSRTCom() {
    mInterpolation = static_cast<KeyTrackBase::Interpolation>(KeyTrackBase::kInterpolationLinear | 0x40000000);
}

// Reconstructed from eboot.elf at 0x31E620. The struct registration is
// RegisterPropStruct<TKey<TransformSRTKey>> (0x31ED00) and the base
// properties _InitPropKeysBase<PropKeysTransformSRTCom> (0x31EEB0). The
// key's "value" is the transform, read and written through the accessors;
// the key itself is saved as "serialized_value".
void PropKeysTransformSRTCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<TransformSRTKey>>(Symbol("TKey<TransformSRTKey>"));
    PropKeysTransformSRTCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysTransformSRTCom, mKeys), sizeof(PropArray<TKey<TransformSRTKey>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<TransformSRTKey>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<TransformSRTKey> keyPrototype = {0.0f, TransformSRTKey()};
    key.mHelp = String("A TransformSRT Key in the PropAnim!");
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<TransformSRTKey>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    const PropXfm valueType = {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
    XfmMetadata& value = TypeSpecificMetadata(key.mRegistry.RegisterProp("value", -1, kPropertyXfm, 0), valueType);
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    // At 0x321900 and 0x321980.
    value.mGetter = [](const PropAccessorArgs& args) {
        const TransformSRTKey& transform = AccessedKey<TransformSRTKey>(args);
        transform.UpdateEuler();
        return transform.mXfm;
    };
    value.mSetter = [](const PropAccessorArgs& args) {
        TransformSRTKey& transform = AccessedKey<TransformSRTKey>(args);
        transform.mXfm = *static_cast<const PropXfm*>(args.mValue);
        transform.mFlags &= ~RotationKey::kEulerStale;
        transform.mQuat.Set(transform.mXfm.mRotation);
        transform.mFlags &= ~RotationKey::kQuatStale;
    };

    PropRegistry& serialized = static_cast<GroupMetadata&>(
                                   *key.mRegistry
                                        .RegisterProp(
                                            "serialized_value", offsetof(TKey<TransformSRTKey>, mValue),
                                            kPropertyStruct, sizeof(TransformSRTKey))
                                        .mMetadata.Get())
                                   .mRegistry;
    static_cast<void>(Symbol("struct"));
    const TransformSRTKey serializedPrototype;
    XfmMetadata& transform = TypeSpecificMetadata(
        serialized.RegisterProp("transform", offsetof(TransformSRTKey, mXfm), kPropertyXfm, sizeof(PropXfm)),
        serializedPrototype.mXfm);
    transform.mDefault = serializedPrototype.mXfm;
    PropRegistry& quat = static_cast<GroupMetadata&>(
                             *serialized
                                  .RegisterProp("quat", offsetof(TransformSRTKey, mQuat), kPropertyStruct, sizeof(Hmx::Quat))
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
        serialized.RegisterProp("flags", offsetof(TransformSRTKey, mFlags), kPropertyUInt8, 1), flagsPrototype);
    flags.mDefault = flagsPrototype;
    _InitPropKeysBase<PropKeysTransformSRTCom>(registry);
    metadata.mDescription = "Com that contains the TransformSRT PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x320020.
Symbol PropKeysTransformSRTCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x320030.
Symbol PropKeysTransformSRTCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x320040.
int PropKeysTransformSRTCom::CurrentRev() const {
    return const_cast<PropKeysTransformSRTCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x320060.
bool PropKeysTransformSRTCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x320090.
Component* PropKeysTransformSRTCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x3200A0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysTransformSRTCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysTransformSRTCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysTransformSRTCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x3202D0.
PropRegistry& PropKeysTransformSRTCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x3202E0.
ComMetaData& PropKeysTransformSRTCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x320540.
PropertyType PropKeysTransformSRTCom::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x320550.
Symbol PropKeysTransformSRTCom::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x3205F0.
bool PropKeysTransformSRTCom::IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
    static_cast<void>(interpolation);
    return true;
}

// Reconstructed from eboot.elf at 0x320600.
bool PropKeysTransformSRTCom::IsRotationKeys() const {
    return true;
}

// Reconstructed from eboot.elf at 0x320610.
bool PropKeysTransformSRTCom::HasSerializedValue() const {
    return true;
}

// Reconstructed from eboot.elf at 0x3209E0. The scale, the angles and the
// translation, in that order.
DataNode PropKeysTransformSRTCom::_ValueToDataNode(const TransformSRTKey& value, bool forEditor) const {
    static_cast<void>(forEditor);
    value.UpdateEuler();
    DataArray* const array = new DataArray(9);
    array->Node(0) = DataNode(value.mXfm.mScale.x);
    array->Node(1) = DataNode(value.mXfm.mScale.y);
    array->Node(2) = DataNode(value.mXfm.mScale.z);
    array->Node(3) = DataNode(value.mXfm.mRotation.x);
    array->Node(4) = DataNode(value.mXfm.mRotation.y);
    array->Node(5) = DataNode(value.mXfm.mRotation.z);
    array->Node(6) = DataNode(value.mXfm.mTranslation.x);
    array->Node(7) = DataNode(value.mXfm.mTranslation.y);
    array->Node(8) = DataNode(value.mXfm.mTranslation.z);
    return DataNode(array, kDataArray);
}

// Reconstructed from eboot.elf at 0x321120. The node's array is read once
// and dropped before the nine reads.
TransformSRTKey PropKeysTransformSRTCom::_ValueFromDataNode(const DataNode& value) const {
    static_cast<void>(value.Array(nullptr));
    const float scaleX = value.Array(nullptr)->Float(0);
    const float scaleY = value.Array(nullptr)->Float(1);
    const float scaleZ = value.Array(nullptr)->Float(2);
    const float pitch = value.Array(nullptr)->Float(3);
    const float roll = value.Array(nullptr)->Float(4);
    const float yaw = value.Array(nullptr)->Float(5);
    const float x = value.Array(nullptr)->Float(6);
    const float y = value.Array(nullptr)->Float(7);
    const float z = value.Array(nullptr)->Float(8);
    const PropXfm xfm = {{scaleX, scaleY, scaleZ}, {pitch, roll, yaw}, {x, y, z}};
    return TransformSRTKey(xfm);
}

// Reconstructed from eboot.elf at 0x321560.
TransformSRTKey PropKeysTransformSRTCom::_DefaultValue() const {
    return TransformSRTKey();
}

// Reconstructed from eboot.elf at 0x31F810.
template <>
void PropKeysTypeCom<TransformSRTKey>::UpdateValueAtIndex(unsigned long index, Entity* entity) {
    KeyAt(index).mValue = TransformSRTKey(mProp.Get<PropXfm>(entity));
}

// Reconstructed from eboot.elf at 0x31F450.
template <>
unsigned long PropKeysTypeCom<TransformSRTKey>::AddNewKey(float frame, Entity* entity, bool replace) {
    return this->AddKey(TransformSRTKey(mProp.Get<PropXfm>(entity)), frame, replace, false);
}

// Reconstructed from eboot.elf at 0x31F8F0. The sample's Euler angles are
// refreshed before the property is set.
template <>
bool PropKeysTypeCom<TransformSRTKey>::ApplyFrame(float frame, Entity* entity) const {
    TransformSRTKey value;
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
    if (info != nullptr && _SetProp(*component, mProp.mPath, *info, storage, &value.mXfm, kPropertyXfm, false)) {
        return true;
    }
    static_cast<void>(component->MakeErrorName());
    char path[512] = {};
    mProp.mPath.ToString(path, sizeof(path));
    return false;
}

// Reconstructed from eboot.elf at 0x31FD70. The binary takes the keys'
// PropRef.
template <>
void TPropDriver<PropKeysTransformSRTCom>::_SetDrivenProperty(Component* component, const PropPath& path, const void* value) {
    const TransformSRTKey& key = *static_cast<const TransformSRTKey*>(value);
    key.UpdateEuler();
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _SetProp(*component, path, *info, storage, &key.mXfm, kPropertyXfm, false)) {
        return;
    }
    static_cast<void>(component->MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

// Reconstructed from eboot.elf at 0x2B3470. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysTransformSRTCom::_Create() {
    return new PropKeysTransformSRTCom();
}

// Reconstructed from eboot.elf at 0x2AB800. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysTransformSRTCom::Init() {
    InitComponentClass<PropKeysTransformSRTCom>();
}
