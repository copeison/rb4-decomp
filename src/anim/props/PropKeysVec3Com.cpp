// anim/PropKeysVec3Com.o (0x322340-0x32531F). The class's registration and
// factory are emitted in anim/AnimInit.o.
#include "anim/props/PropKeysVec3Com.h"

#include <new>

#include "anim/props/PropAnimResource.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataUtl.h"

// The object's statics, in the order of its static initializer (0x325260).
Symbol PropKeysVec3Com::sId("PropKeysVec3Com");
Symbol PropKeysVec3Com::sClassName("PropKeysVec3Com");
PropRegistry PropKeysVec3Com::sPropRegistry;
ComMetaData PropKeysVec3Com::sMetaData;

// Reconstructed from eboot.elf at 0x322340.
PropDriver* PropKeysVec3Com::StaticCreatePropDriver(const PropRef& prop) {
    return new TPropDriver<PropKeysVec3Com>(prop);
}

// Reconstructed from eboot.elf at 0x322410.
void PropKeysVec3Com::Register() {
    const Symbol className = sClassName;
    RegisterFactory(StaticKeysType(), StaticKeysSubType(), className, StaticCreatePropDriver);
    sMetaData.mInterface = Symbol("PropKeys");
}

// Reconstructed from eboot.elf at 0x3224F0.
PropDriver* PropKeysVec3Com::CreatePropDriver() const {
    return new TPropDriver<PropKeysVec3Com>(mProp);
}

// Reconstructed from eboot.elf at 0x3225C0.
const char* PropKeysVec3Com::StaticKeyTypeName() {
    return "TKey<kVector3>";
}

// Reconstructed from eboot.elf at 0x3225D0. The struct registration is
// RegisterPropStruct<TKey<Vector3>> (0x322A10) and the base properties
// _InitPropKeysBase<PropKeysVec3Com> (0x322BC0).
void PropKeysVec3Com::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<TKey<Vector3>>(Symbol("TKey<Vector3>"));
    PropKeysVec3Com prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    PropInfo& keys = registry.RegisterPropArray(
        "keys", offsetof(PropKeysVec3Com, mKeys), sizeof(PropArray<TKey<Vector3>>));
    ArrayMetadata& keysMetadata = static_cast<ArrayMetadata&>(*keys.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(keys, kPropertyStruct, sizeof(TKey<Vector3>));
    GroupMetadata& key = static_cast<GroupMetadata&>(*keysMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TKey<Vector3> keyPrototype = {};
    FloatMetadata& frame = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("frame", offsetof(TKey<Vector3>, mFrame), kPropertyFloat, 4),
        keyPrototype.mFrame);
    frame.mDefault = keyPrototype.mFrame;
    Vector3Metadata& value = TypeSpecificMetadata(
        key.mRegistry.RegisterProp("value", offsetof(TKey<Vector3>, mValue), kPropertyVector3, sizeof(Vector3)),
        keyPrototype.mValue);
    value.mDefault = keyPrototype.mValue;
    static_cast<void>(Symbol("prop"));
    RemapPropMetadata(value, sRemapKeyValue);
    key.mHelp = String("A Vector3 Key in the PropAnim!");
    _InitPropKeysBase<PropKeysVec3Com>(registry);
    metadata.mDescription = "Com that contains the Vector3 PropAnim keys for a single driven Property";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryAnimation;
    metadata.mAllowedResources.push_back(PropAnimResource::Id());
    Register();
}

// Reconstructed from eboot.elf at 0x3231F0.
Symbol PropKeysVec3Com::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x323200.
Symbol PropKeysVec3Com::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x323210.
int PropKeysVec3Com::CurrentRev() const {
    return const_cast<PropKeysVec3Com*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x323230.
bool PropKeysVec3Com::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x323260.
Component* PropKeysVec3Com::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x323270. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* PropKeysVec3Com::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<PropKeysVec3Com*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) PropKeysVec3Com(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x3234A0.
PropRegistry& PropKeysVec3Com::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x3234B0.
ComMetaData& PropKeysVec3Com::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x323710.
PropertyType PropKeysVec3Com::GetKeysType() const {
    return StaticKeysType();
}

// Reconstructed from eboot.elf at 0x323720.
Symbol PropKeysVec3Com::GetKeysSubType() const {
    return StaticKeysSubType();
}

// Reconstructed from eboot.elf at 0x3237C0.
bool PropKeysVec3Com::IsValidInterpolation(KeyTrackBase::Interpolation interpolation) const {
    static_cast<void>(interpolation);
    return true;
}

// Reconstructed from eboot.elf at 0x324150.
DataNode PropKeysVec3Com::_ValueToDataNode(const Vector3& value, bool forEditor) const {
    static_cast<void>(forEditor);
    return DataNode(MakeDataArray(DataNode(value.x), DataNode(value.y), DataNode(value.z)), kDataArray);
}

// Reconstructed from eboot.elf at 0x3242C0. The node's array is read once
// and dropped before the three reads.
Vector3 PropKeysVec3Com::_ValueFromDataNode(const DataNode& value) const {
    static_cast<void>(value.Array(nullptr));
    return Vector3{value.Array(nullptr)->Float(0), value.Array(nullptr)->Float(1), value.Array(nullptr)->Float(2)};
}

// Reconstructed from eboot.elf at 0x324440.
Vector3 PropKeysVec3Com::_DefaultValue() const {
    return Vector3{0.0f, 0.0f, 0.0f};
}

// Reconstructed from eboot.elf at 0x2B2B00. The binary emits the factory in
// anim/AnimInit.o with the class's Init.
Component* PropKeysVec3Com::_Create() {
    return new PropKeysVec3Com();
}

// Reconstructed from eboot.elf at 0x2AAF10. The binary emits the
// registration in anim/AnimInit.o, which calls it.
void PropKeysVec3Com::Init() {
    InitComponentClass<PropKeysVec3Com>();
}
